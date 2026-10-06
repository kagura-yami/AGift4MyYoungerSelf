"""Corridor patrol only; preserve all existing level geometry."""
import unreal,shutil
from pathlib import Path
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='lv4'
root=Path(unreal.Paths.project_dir()).resolve();backup=root.parent/'tools/evaluation/lv4-before-corridor-patrol.umap'
if not backup.exists():shutil.copy2(root/'Content/Maps/lv4.umap',backup)
def spawn(cls,name,xyz):
 a=next((a for a in aa.get_all_level_actors() if a.get_actor_label()==name),None)
 if not a:a=aa.spawn_actor_from_class(cls,unreal.Vector(*xyz))
 a.modify();a.set_actor_location(unreal.Vector(*xyz),False,True);a.set_actor_label(name);a.set_folder_path('Gameplay/CorridorPatrol');return a
points=[spawn(unreal.TargetPoint,'LV4_Patrol_End_'+str(i),(1956,y,610)) for i,y in enumerate([1050,-2030])]
npc=spawn(unreal.TripoChaser,'LV4_CorridorPatrol',(1956,1050,610))
npc.set_actor_rotation(unreal.Rotator(yaw=-90),False)
for key,value in [('bRestrictToPatrolArea',True),('patrol_area_center',unreal.Vector(1956,-470,700)),('patrol_area_extent',unreal.Vector(175,1610,300)),('patrol_points',points),('bStartPatrolling',True),('bPingPongPatrol',True),('patrol_wait_seconds',1.5),('sight_radius',850),('sight_half_angle',55),('close_detection_radius',0),('search_speed_ratio',.35),('lost_aggro_per_second',25),('bShowDebug',False)]:npc.set_editor_property(key,value)
# Use the project's existing humanoid and locomotion assets for visible walking.
source=unreal.get_default_object(unreal.TripoCharacter).get_component_by_class(unreal.SkeletalMeshComponent)
m=npc.get_component_by_class(unreal.SkeletalMeshComponent)
m.set_skeletal_mesh_asset(source.get_editor_property('skeletal_mesh_asset'));m.set_anim_instance_class(source.get_editor_property('anim_class'))
m.set_relative_location(unreal.Vector(0,0,-88),False,True);m.set_relative_rotation(unreal.Rotator(yaw=-90),False,True);m.set_relative_scale3d(unreal.Vector(.7,.7,.7))
npc.whitebox_body.set_hidden_in_game(True)
nav=next(a for a in aa.get_all_level_actors() if a.get_actor_label()=='LV4_OfficeChase_Navigation')
nav.set_actor_location(unreal.Vector(200,-600,700),False,True);nav.set_actor_scale3d(unreal.Vector(20,20,2))
for a in list(aa.get_all_level_actors()):
 if a.get_actor_label() in ['LV4_Patrol_Checkpoint_Entry','LV4_Patrol_Checkpoint_Exit']:aa.destroy_actor(a)
next(a for a in aa.get_all_level_actors() if isinstance(a,unreal.TripoChaseTrigger)).build_navigation_for_level()
assert ls.save_current_level()
print('CORRIDOR_PATROL_SAVED')
