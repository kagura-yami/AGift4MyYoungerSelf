"""Place the existing UpDash system instruction on the Home corridor wall."""
import unreal
import shutil
from pathlib import Path
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world and world.get_name()=='L_Home_Vertical_02'
assert not unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()
root=Path(unreal.Paths.project_dir()).resolve()
backup=root.parent/'tools/evaluation/home-before-wall-notice.umap'
if not backup.exists(): shutil.copy2(root/'Content/Maps/HomeVertical/Maps/L_Home_Vertical_02.umap',backup)
trigger=next(a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
             if isinstance(a,unreal.TripoStoryTrigger) and str(a.event_id)=='Story.Home.GrantUpward')
start=trigger.get_actor_location()+unreal.Vector(0,0,170)
hit=unreal.SystemLibrary.line_trace_single(world,start,start+unreal.Vector(0,-700,0),unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,False,[trigger],unreal.DrawDebugTrace.NONE,True).to_tuple()
assert hit[0] and hit[9].get_name()=='StaticMeshActor_2', 'Expected corridor wall missing'
trigger.modify()
trigger.set_editor_property('bWallNotice',True)
notice=trigger.get_component_by_class(unreal.WidgetComponent)
notice.set_relative_scale3d(unreal.Vector(.35,.35,.35))
notice.set_world_location(hit[5]+hit[7]*2,False,True)
notice.set_world_rotation(unreal.Rotator(pitch=0,yaw=90,roll=0),False,True)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('WALL_NOTICE_READY',notice.get_world_location())
