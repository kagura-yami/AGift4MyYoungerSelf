"""Replace the two Home gift props with persistent ability rewards, retaining their art."""
import shutil
from pathlib import Path
import unreal

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert world and world.get_name() == 'L_Home_Vertical_02'
assert not unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages(), 'Save pending map edits first'
root = Path(unreal.Paths.project_dir()).resolve()
backup = root.parent / 'tools/evaluation/home-gifts-backup.umap'
if not backup.exists():
    shutil.copy2(root / 'Content/Maps/HomeVertical/Maps/L_Home_Vertical_02.umap', backup)
existing = {a.get_name(): a for a in actors.get_all_level_actors()}
for source, label, key, fixed in [
    ('StaticMeshActor_376', 'Home_Gift_Bedside_Random', 'Home.Bedside.Random', False),
    ('StaticMeshActor_410', 'Home_Gift_LivingRoom_UpDash', 'Home.LivingRoom.UpDash', True),
]:
    if any(a.get_actor_label() == label for a in actors.get_all_level_actors()):
        continue
    original = existing[source]
    mesh = original.get_component_by_class(unreal.StaticMeshComponent)
    gift = actors.spawn_actor_from_class(unreal.TripoGiftBox, original.get_actor_location(), original.get_actor_rotation())
    gift.set_actor_label(label)
    gift.set_folder_path('Gameplay/Gifts')
    gift.set_editor_property('gift_id', key)
    gift.box_mesh.set_static_mesh(mesh.static_mesh)
    gift.box_mesh.set_relative_location(unreal.Vector(), False, True)
    gift.box_mesh.set_relative_scale3d(original.get_actor_scale3d())
    gift.box_mesh.set_editor_property('override_materials', mesh.get_editor_property('override_materials'))
    gift.lid_mesh.set_static_mesh(None)
    gift.prompt.set_hidden_in_game(True)
    focus = gift.get_component_by_class(unreal.TripoInteractionTarget)
    focus.set_relative_location(unreal.Vector(0, 0, 25), False, True)
    focus.set_box_extent(unreal.Vector(25, 26, 25))
    focus.set_editor_property('prompt', '打开礼物 · 上位移' if fixed else '打开礼物 · 随机能力')
    if fixed:
        reward = unreal.TripoRewardOption()
        reward.set_editor_property('ability', unreal.TripoAbility.UP_DASH)
        reward.set_editor_property('weight', 1)
        gift.set_editor_property('rewards', [reward])
    assert actors.destroy_actor(original)
    print(label, gift.get_actor_location(), len(gift.rewards))
assert levels.save_current_level()
print('HOME_GIFTS_READY')
