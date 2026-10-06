"""Equip LV4 pursuers and open the end elevator; preserve source elevator art."""
import json, shutil, time
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve().parent
es = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
ls = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert es.get_editor_world().get_name() == 'lv4' and es.get_game_world() is None
assert ls.save_current_level()
backup = root / 'tools/evaluation/grandma-chase' / time.strftime('%Y%m%d-%H%M%S')
backup.mkdir(parents=True, exist_ok=True)
paths = ['Maps/lv4.umap', 'Blueprint/Lv4/BP_OfficeDoorChaser.uasset',
         'Models/juese/SK_MiniCharacter_Grandma_01.uasset',
         'Models/juese/SK_MiniCharacter_Grandma_01_Skeleton.uasset',
         'Models/juese/SK_MiniCharacter_Son_01_Skeleton.uasset']
for p in paths:
    target = backup / p
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(root / 'game/Content' / p, target)

grandma = unreal.load_asset('/Game/Models/juese/SK_MiniCharacter_Grandma_01')
son = unreal.load_asset('/Game/Models/juese/SK_MiniCharacter_Son_01')
gp, sp = grandma.skeleton.get_reference_pose(), son.skeleton.get_reference_pose()
assert list(gp.get_bone_names()) == list(sp.get_bone_names())
for b in gp.get_bone_names():
    a, c = gp.get_bone_pose(b), sp.get_bone_pose(b)
    assert a.translation.distance(c.translation) < .001
    assert abs(abs(sum(getattr(a.rotation, k) * getattr(c.rotation, k) for k in 'xyzw')) - 1) < .0001
grandma.skeleton.add_compatible_skeleton(son.skeleton)
son.skeleton.add_compatible_skeleton(grandma.skeleton)
mat_path = '/Game/Models/juese/MI_Grandma_Chase'
mat = unreal.load_asset(mat_path) or unreal.EditorAssetLibrary.duplicate_asset('/Game/Models/lv4/caizhi1_shili', mat_path)
slots = list(grandma.materials)
slots[0].material_interface = mat
grandma.set_editor_property('materials', slots)
for asset in [mat, grandma, grandma.skeleton, son.skeleton]:
    asset.modify()
    assert unreal.EditorAssetLibrary.save_loaded_asset(asset, False)

def equip(npc):
    npc.modify()
    mesh = npc.get_component_by_class(unreal.SkeletalMeshComponent)
    mesh.modify()
    mesh.set_skeletal_mesh_asset(grandma)
    mesh.set_material(0, mat)
    mesh.set_relative_location(unreal.Vector(0, 0, -88), False, True)
    mesh.set_relative_rotation(unreal.Rotator(yaw=-90), False, True)
    mesh.set_relative_scale3d(unreal.Vector(.85, .85, .85))
    mesh.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    mesh.set_anim_instance_class(unreal.TripoLocomotionAnimInstance)
    npc.whitebox_body.set_hidden_in_game(True)
    npc.whitebox_body.set_visibility(False)
    npc.set_editor_property('bShowDebug', False)

bp = unreal.load_asset('/Game/Blueprint/Lv4/BP_OfficeDoorChaser')
cls = unreal.load_object(None, '/Game/Blueprint/Lv4/BP_OfficeDoorChaser.BP_OfficeDoorChaser_C')
equip(unreal.get_default_object(cls))
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert unreal.EditorAssetLibrary.save_loaded_asset(bp)
by = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
equip(by['LV4_CorridorPatrol'])

# Give the concave cabin explicit triangle collision without altering other elevators.
elevator = by['SM_Bld_Elevator_2']
mesh_path = '/Game/Blueprint/Lv4/Collision/SM_ChaseEndElevator_Open'
mesh = unreal.load_asset(mesh_path) or unreal.EditorAssetLibrary.duplicate_asset('/Game/Models/lv4/SM_Bld_Elevator_01', mesh_path)
unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem).remove_collisions(mesh)
body = mesh.get_editor_property('body_setup')
body.set_editor_property('collision_trace_flag', unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
body.set_editor_property('double_sided_geometry', True)
assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
elevator.modify()
elevator.static_mesh_component.set_static_mesh(mesh)
elevator.static_mesh_component.set_collision_profile_name('BlockAll')

# Pawn is ignored, while the dedicated Chaser channel is blocked.
name = 'LV4_ChaseEndElevator_SafeZone'
zone = by.get(name) or actors.spawn_actor_from_class(unreal.TripoChaseHideZone, unreal.Vector(2280, -2300, 715))
zone.modify()
zone.set_actor_label(name)
zone.set_folder_path('Gameplay/OfficeChase')
zone.set_actor_location(unreal.Vector(2280, -2300, 715), False, True)
zone.volume.set_box_extent(unreal.Vector(150, 135, 175))
zone.volume.set_collision_response_to_all_channels(unreal.CollisionResponseType.ECR_IGNORE)
zone.volume.set_collision_response_to_channel(unreal.CollisionChannel.ECC_TRIPO_CHASER, unreal.CollisionResponseType.ECR_BLOCK)
zone.set_editor_property('aggro_reduction_per_second', 0)
zone.set_enabled(True)
zone.label.set_hidden_in_game(True)
zone.label.set_visibility(False)
by['LV4_OfficeDoor_ChaseTrigger'].build_navigation_for_level()
assert ls.save_current_level()
print('GRANDMA_CHASE_SAVED', str(backup))
