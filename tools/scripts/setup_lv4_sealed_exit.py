"""Seal the ground-floor emergency exit with a non-interactive door leaf."""
import unreal,shutil
from pathlib import Path
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='lv4'
root=Path(unreal.Paths.project_dir()).resolve()
backup=root.parent/'tools/evaluation/lv4-before-sealed-exit.umap'
if not backup.exists():shutil.copy2(root/'Content/Maps/lv4.umap',backup)
actors=aa.get_all_level_actors()
def actor(name):
    a=next((a for a in actors if a.get_actor_label()==name),None)
    if not a:a=aa.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
    a.set_actor_label(name);a.set_folder_path('LV4/GroundFloor/SealedExit')
    return a
door=actor('LV4_SealedEmergencyDoor')
door.static_mesh_component.set_static_mesh(unreal.load_asset('/Game/TripoModels/rusted_green_door_3d_model/rusted_green_door_3d_model'))
door.set_actor_location(unreal.Vector(540,105.85,212),False,True)
door.set_actor_rotation(unreal.Rotator(),True)
door.set_actor_scale3d(unreal.Vector(1,1.54,1.443))
door.static_mesh_component.set_collision_profile_name('BlockAll')
block=actor('LV4_SealedEmergencyDoor_Blocker')
block.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'))
block.set_actor_location(unreal.Vector(540,30,212),False,True)
block.set_actor_scale3d(unreal.Vector(.14,1.52,3.04))
block.static_mesh_component.set_collision_profile_name('BlockAll')
block.set_actor_hidden_in_game(True);block.set_is_temporarily_hidden_in_editor(True)
assert len(door.get_components_by_class(unreal.TripoInteractionTarget))==0
assert ls.save_current_level()
print('Sealed exit saved; no opening Blueprint or interaction component.')

