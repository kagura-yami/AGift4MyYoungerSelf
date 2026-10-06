"""Replace the Home chapter-end prop, preserving the room and disabling its old exit."""
import unreal,shutil,json
from pathlib import Path
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert es.get_editor_world() and es.get_editor_world().get_name()=='L_Home_Vertical_02' and not es.get_game_world()
root=Path(unreal.Paths.project_dir()).resolve()
backup=root.parent/'tools/evaluation/home-before-chapter-choice.umap'
if not backup.exists():shutil.copy2(root/'Content/Maps/HomeVertical/Maps/L_Home_Vertical_02.umap',backup)
actors=aa.get_all_level_actors()
existing=next((a for a in actors if isinstance(a,unreal.TripoChapterGift)),None)
if existing:g=existing
else:
    marker=next(a for a in actors if isinstance(a,unreal.TripoStoryTrigger) and str(a.event_id)=='Story.Home.GiftFound')
    candidates=[]
    for a in actors:
        if isinstance(a,unreal.TripoGiftBox):candidates.append(a)
        elif isinstance(a,unreal.StaticMeshActor):
            mesh=a.static_mesh_component.static_mesh
            if mesh and 'gift_box' in mesh.get_name().lower():candidates.append(a)
    source=min(candidates,key=lambda a:(a.get_actor_location()-marker.get_actor_location()).length())
    assert (source.get_actor_location()-marker.get_actor_location()).length()<700,'No end-room gift found nearby'
    print('REPLACE',source.get_actor_label(),source.get_actor_location())
    g=aa.spawn_actor_from_class(unreal.TripoChapterGift,source.get_actor_location(),source.get_actor_rotation())
    # Keep the portal in centimetres regardless of the imported prop scale.
    scale=source.get_actor_scale3d()
    for comp,path in [(g.box_mesh,'/Game/Blueprint/Gift/SM_HomeGiftBody'),(g.lid_mesh,'/Game/Blueprint/Gift/SM_HomeGiftLid')]:
        comp.set_static_mesh(unreal.load_asset(path));comp.set_relative_location(unreal.Vector(),False,True);comp.set_relative_scale3d(scale)
    aa.destroy_actor(source)
g.set_actor_label('Home_ChapterGift_ChooseLevel2');g.set_folder_path('Gameplay/Gifts')
g.set_editor_property('gift_id','Home.ChapterEnd.ChooseLevel2')
g.set_editor_property('destination','/Game/Maps/School/School_v2')
g.prompt.set_hidden_in_game(True)
g.portal.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Plane'))
g.portal.set_material(0,unreal.load_asset('/Game/Materials/Portal/M_ChapterPortal'))
g.portal.set_world_location(unreal.Vector(4943,400,7890),False,True)
g.portal.set_world_rotation(unreal.Rotator(pitch=0,yaw=90,roll=90),False,True)
f=g.get_component_by_class(unreal.TripoInteractionTarget)
f.set_relative_location(unreal.Vector(0,0,30),False,True);f.set_box_extent(unreal.Vector(35,35,35))
f.set_editor_property('prompt','打开终点礼物 · 三选一')
for a in actors:
    if isinstance(a,unreal.TripoStoryTrigger) and str(a.event_id) in ['Story.Home.Exit','Story.Home.GiftFound']:
        aa.destroy_actor(a)
safe=next((a for a in aa.get_all_level_actors() if a.get_actor_label()=='Safe_Home_ChapterPortal'),None)
if not safe:safe=aa.spawn_actor_from_class(unreal.TripoZone,g.get_actor_location()+unreal.Vector(0,0,100))
safe.set_actor_label('Safe_Home_ChapterPortal');safe.set_folder_path('Gameplay/Checkpoints')
safe.set_editor_property('kind',unreal.TripoZoneKind.SAFE)
safe.set_editor_property('bAutoCheckpoint',True)
safe.set_actor_location(unreal.Vector(5350,400,7890),False,True)
safe.volume.set_box_extent(unreal.Vector(480,350,180))
safe.set_editor_property('hint','')
assert ls.save_current_level()
print('CHAPTER_GIFT_READY',g.get_actor_location())

