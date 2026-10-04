import unreal, shutil
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
assert es.get_editor_world().get_name()=='lv4' and not es.get_game_world()
shutil.copy2(root/'game/Content/Maps/lv4.umap',root/'tools/evaluation/lv4-before-roof-gift.umap')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
by={a.get_actor_label():a for a in actors.get_all_level_actors()}
e=by['LV4_WestElevator']
shell=by['SM_Bld_Elevator_01']
hit=unreal.SystemLibrary.line_trace_single(es.get_editor_world(),unreal.Vector(-1790,650,540),unreal.Vector(-1790,650,350),unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,True,[],unreal.DrawDebugTrace.NONE,True).to_tuple()
assert hit[0] and hit[9]==shell
g=by.get('LV4_Gift_ElevatorRoof_Random') or actors.spawn_actor_from_class(unreal.TripoGiftBox,hit[5])
g.modify(); g.set_actor_label('LV4_Gift_ElevatorRoof_Random'); g.set_folder_path('LV4_Mechanisms/WestElevator')
g.set_actor_location(hit[5]+unreal.Vector(0,0,.3),False,True)
g.set_editor_property('gift_id','LV4.ElevatorRoof.Random')
g.set_editor_property('open_seconds',1.0)
g.box_mesh.set_static_mesh(unreal.load_asset('/Game/Blueprint/Gift/SM_HomeGiftBody'))
g.lid_mesh.set_static_mesh(unreal.load_asset('/Game/Blueprint/Gift/SM_HomeGiftLid'))
for c in [g.box_mesh,g.lid_mesh]:
    c.set_relative_location(unreal.Vector(),False,True)
    c.set_relative_scale3d(unreal.Vector(1,1,1))
g.prompt.set_hidden_in_game(True)
t=g.get_component_by_class(unreal.TripoInteractionTarget)
t.set_relative_location(unreal.Vector(0,0,25),False,True); t.set_box_extent(unreal.Vector(25,26,25))
g.attach_to_component(e.cabin,unreal.Name('None'),unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
assert len(g.rewards)==8
for name in ['LV4_UpperDoorDashEntry','LV4_UpperDoorDashExit']:
    point=by[name]; point.modify(); point.set_editor_property('required_floor',0)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('Roof gift placed',g.get_actor_location(),'8 rewards, floor-0 dash requirement')
