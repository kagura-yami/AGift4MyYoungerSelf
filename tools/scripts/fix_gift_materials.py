"""Restore the existing wrapping-paper material on playable chapter gifts."""
import unreal

material = unreal.load_asset('/Game/Materials/Gift/MI_Gift')
assert material
# Keep the release material independent of an editor import-plugin texture.
unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
    material, 'Metallicity', unreal.load_asset('/Game/TripoModels/Materials/T_Tripo_Default_Black_Linear'))
unreal.MaterialEditingLibrary.update_material_instance(material)
unreal.EditorAssetLibrary.save_loaded_asset(material, False)
for name in ['Gift_Box', 'Gift_Lid', 'SM_HomeGiftBody', 'SM_HomeGiftLid']:
    mesh = unreal.load_asset('/Game/Blueprint/Gift/' + name)
    assert mesh
    mesh.modify()
    mesh.set_material(0, material)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh, False)

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
def fix_level(path):
    assert levels.load_level(path)
    count = 0
    for actor in actors.get_all_level_actors():
        if isinstance(actor, unreal.TripoGiftBox):
            actor.modify()
            for component in [actor.box_mesh, actor.lid_mesh]:
                component.set_material(0, material)
                assert component.get_material(0) == material
            count += 1
    assert levels.save_current_level()
    print('GIFT_MATERIAL_FIXED', path, count)

for path in ['/Game/Maps/HomeVertical/Maps/L_Home_Vertical_02', '/Game/Maps/School/School_v2', '/Game/Maps/lv4']:
    fix_level(path)
