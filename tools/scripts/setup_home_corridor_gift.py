import unreal, shutil
from pathlib import Path
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert es.get_editor_world().get_name()=='L_Home_Vertical_02' and not es.get_game_world()
root=Path(unreal.Paths.project_dir()).resolve()
shutil.copy2(root/'Content/Maps/HomeVertical/Maps/L_Home_Vertical_02.umap',root.parent/'tools/evaluation/home-before-corridor-dash-gift.umap')
source=next(a for a in aa.get_all_level_actors() if a.get_actor_label()=='gift_box_3d_model_2')
g=aa.spawn_actor_from_class(unreal.TripoGiftBox,source.get_actor_location(),source.get_actor_rotation())
g.set_actor_label('Home_Gift_Corridor_Dash'); g.set_folder_path('Gameplay/Gifts')
g.set_actor_scale3d(source.get_actor_scale3d()); g.set_editor_property('gift_id','Home.Corridor.Dash')
for component,path in [(g.box_mesh,'/Game/Blueprint/Gift/SM_HomeGiftBody'),(g.lid_mesh,'/Game/Blueprint/Gift/SM_HomeGiftLid')]:
    asset=unreal.load_asset(path); assert asset
    component.set_static_mesh(asset)
    component.set_relative_location(unreal.Vector(),False,True)
    component.set_relative_scale3d(unreal.Vector(1,1,1))
r=unreal.TripoRewardOption(); r.set_editor_property('ability',unreal.TripoAbility.DASH); r.set_editor_property('weight',1)
g.set_editor_property('rewards',[r]); g.prompt.set_hidden_in_game(True)
f=g.get_component_by_class(unreal.TripoInteractionTarget)
f.set_relative_location(unreal.Vector(0,0,25),False,True); f.set_box_extent(unreal.Vector(25,26,25))
f.set_editor_property('prompt','打开礼物 · 平面位移')
assert len(g.rewards)==1 and g.rewards[0].ability==unreal.TripoAbility.DASH
assert aa.destroy_actor(source)
aa.set_selected_level_actors([g])
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('PASS: corridor fixed DASH reward, animated gift, saved',g.get_actor_location())
