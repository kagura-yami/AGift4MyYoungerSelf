"""Wire the office doorway to the existing whitebox chase system."""
import unreal,shutil
from pathlib import Path
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
assert es.get_editor_world().get_name()=='lv4'
root=Path(unreal.Paths.project_dir()).resolve()
backup=root.parent/'tools/evaluation/lv4-before-office-chase.umap'
if not backup.exists():shutil.copy2(root/'Content/Maps/lv4.umap',backup)
def spawn(cls,name,loc):
 a=next((a for a in aa.get_all_level_actors() if a.get_actor_label()==name),None)
 if not a:a=aa.spawn_actor_from_class(cls,unreal.Vector(*loc))
 a.set_actor_label(name);a.set_folder_path('Gameplay/OfficeChase');return a
bp_path='/Game/Blueprint/Lv4/BP_OfficeDoorChaser'
bp=unreal.load_asset(bp_path)
if not bp:bp=unreal.EditorAssetLibrary.duplicate_asset('/Game/Blueprint/Lv4/BP_Lv4Patrol',bp_path)
cls=unreal.load_object(None,bp_path+'.BP_OfficeDoorChaser_C')
cdo=unreal.get_default_object(cls)
cdo.set_editor_property('bShowDebug',False)
cdo.set_editor_property('bStartPatrolling',False)
cdo.set_editor_property('initial_aggro',100)
cdo.set_editor_property('lost_aggro_per_second',7)
cdo.set_editor_property('sight_radius',1800)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp)
trigger=spawn(unreal.TripoChaseTrigger,'LV4_OfficeDoor_ChaseTrigger',(-1167,-190,610))
trigger.volume.set_box_extent(unreal.Vector(80,45,100))
trigger.set_editor_property('chaser_class',cls)
trigger.set_editor_property('bRearmAfterRestore',True)
point=spawn(unreal.TargetPoint,'LV4_OfficeChaser_Spawn',(-650,-660,610))
point.set_actor_rotation(unreal.Rotator(yaw=180),False)
trigger.set_editor_property('spawn_point',point)
nav=spawn(unreal.NavMeshBoundsVolume,'LV4_OfficeChase_Navigation',(250,-250,700))
nav.set_actor_scale3d(unreal.Vector(18,17,2))
checkpoint=spawn(unreal.TripoZone,'LV4_OfficeChase_Checkpoint',(-1167,55,610))
checkpoint.set_editor_property('kind',unreal.TripoZoneKind.SAFE)
checkpoint.set_editor_property('bAutoCheckpoint',True)
checkpoint.set_editor_property('safe_offset',unreal.Vector(0,0,0))
checkpoint.volume.set_box_extent(unreal.Vector(95,85,95))
checkpoint.get_component_by_class(unreal.TextRenderComponent).set_hidden_in_game(True)
trigger.build_navigation_for_level()
assert ls.save_current_level()
print('OFFICE_CHASE_SAVED',nav.get_actor_bounds(False))

