"""School finale: three level-two choices, then a portal on the CRT screen."""
import unreal,shutil
from pathlib import Path
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert w and w.get_name()=='School_v2'
root=Path(unreal.Paths.project_dir()).resolve()
backup=root.parent/'tools/evaluation/school-before-chapter-portal.umap'
if not backup.exists():shutil.copy2(root/'Content/Maps/School/School_v2.umap',backup)
actors=aa.get_all_level_actors()
g=next((a for a in actors if isinstance(a,unreal.TripoChapterGift) and str(a.gift_id)=='School.ChapterEnd.ChooseLevel2'),None)
if not g:
    source=next(a for a in actors if a.get_actor_label()=='gift_box_3d_model_2')
    g=aa.spawn_actor_from_class(unreal.TripoChapterGift,source.get_actor_location(),source.get_actor_rotation())
    for comp,path in [(g.box_mesh,'/Game/Blueprint/Gift/SM_HomeGiftBody'),(g.lid_mesh,'/Game/Blueprint/Gift/SM_HomeGiftLid')]:
        comp.set_static_mesh(unreal.load_asset(path));comp.set_relative_location(unreal.Vector(),False,True);comp.set_relative_scale3d(source.get_actor_scale3d())
    aa.destroy_actor(source)
g.set_actor_label('School_ChapterGift_ChooseLevel2');g.set_folder_path('Gameplay/Gifts')
g.set_editor_property('gift_id','School.ChapterEnd.ChooseLevel2')
g.set_editor_property('destination','/Game/Maps/lv4')
g.set_editor_property('completion_event','Story.School.GiftFound')
g.set_editor_property('chapter_caption','第二章  /  学校')
g.set_editor_property('exit_hint','收好礼物后，靠近电脑，按 E 进入屏幕里的门')
g.set_editor_property('bScreenPortal',True);g.set_editor_property('portal_scale',.44)
g.prompt.set_hidden_in_game(True)
g.portal.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Plane'))
g.portal.set_material(0,unreal.load_asset('/Game/Materials/Portal/M_ChapterPortal'))
# Trace the actual CRT glass, not the monitor's bounding box or case.
hit=unreal.SystemLibrary.line_trace_single(w,unreal.Vector(26461.36,4220,6330),unreal.Vector(26461.36,4350,6330),unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,True,[g],unreal.DrawDebugTrace.NONE,True).to_tuple()
assert hit[0] and 'crt_monitor' in hit[9].get_actor_label(),hit
surface=hit[5];surface.y-=.4
g.portal.set_world_location(surface,False,True)
g.portal.set_world_rotation(unreal.Rotator(roll=90),False,True)
g.portal.set_relative_scale3d(unreal.Vector(.44,.44,.44))
g.portal_interaction.set_editor_property('prompt','进入传送门 · 第三章')
for f in g.get_components_by_class(unreal.TripoInteractionTarget):
    if f!=g.portal_interaction:
        f.set_relative_location(unreal.Vector(0,0,30),False,True);f.set_box_extent(unreal.Vector(35,35,35));f.set_editor_property('prompt','打开终点礼物 · 三选一')
for a in actors:
    if isinstance(a,unreal.TripoStoryTrigger) and str(a.event_id) in ['Story.School.Exit','Story.School.GiftFound']:aa.destroy_actor(a)
safe=next((a for a in actors if a.get_actor_label()=='Safe_School_ChapterPortal'),None)
if not safe:safe=aa.spawn_actor_from_class(unreal.TripoZone,unreal.Vector(26460,3980,6310))
safe.set_actor_label('Safe_School_ChapterPortal');safe.set_folder_path('Gameplay/Checkpoints')
safe.set_editor_property('kind',unreal.TripoZoneKind.SAFE);safe.set_editor_property('bAutoCheckpoint',True)
safe.volume.set_box_extent(unreal.Vector(150,400,180));safe.set_editor_property('hint','')
assert ls.save_current_level()
print('SCHOOL_PORTAL',surface)
