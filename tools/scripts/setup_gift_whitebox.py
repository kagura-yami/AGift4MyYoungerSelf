"""Create the reward-box Blueprint and add a separate gift lane to the chase whitebox.
Existing Blueprint and placed actors are preserved on rerun.
"""
import unreal

MAP = '/Game/Maps/L_ChaseWhitebox'
BP = '/Game/Blueprint/Gift/BP_RandomGift'
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages():
    raise RuntimeError('Save the current map first; do not discard designer changes.')
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
if world.get_name() != 'L_ChaseWhitebox':
    assert levels.load_level(MAP)
if not unreal.EditorAssetLibrary.does_asset_exist(BP):
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', unreal.TripoGiftBox)
    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset('BP_RandomGift', '/Game/Blueprint/Gift', unreal.Blueprint, factory)
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    cls = unreal.EditorAssetLibrary.load_blueprint_class(BP)
    defaults = unreal.get_default_object(cls)
    defaults.box_mesh.set_static_mesh(unreal.load_asset('/Game/Blueprint/Gift/Gift_Box'))
    defaults.lid_mesh.set_static_mesh(unreal.load_asset('/Game/Blueprint/Gift/Gift_Lid'))
    material = unreal.load_asset('/Game/Materials/Gift/MI_Gift')
    assert material
    defaults.box_mesh.set_editor_property('override_materials', [material])
    defaults.lid_mesh.set_editor_property('override_materials', [material])
    defaults.lid_mesh.set_relative_scale3d(unreal.Vector(.8, .8, .8))
    defaults.lid_mesh.set_relative_location(unreal.Vector(0, 0, 95), False, True)
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(blueprint, False)
cls = unreal.EditorAssetLibrary.load_blueprint_class(BP)
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().modify()

def gift(name, key, xyz, pool=None):
    if name in existing:
        return existing[name]
    box = actors.spawn_actor_from_class(cls, unreal.Vector(*xyz))
    box.set_actor_label(name)
    box.set_editor_property('gift_id', key)
    if pool is not None:
        box.set_editor_property('rewards', pool)
    return box

gift('Gift_Random_AllSkills', 'Whitebox.Random', (0, -450, 0))
stone = unreal.TripoRewardOption()
stone.set_editor_property('ability', unreal.TripoAbility.STEP_STONE)
stone.set_editor_property('weight', 1)
gift('Gift_StepStone_Upgrade', 'Whitebox.Stone', (0, -900, 0), [stone])
gift('Gift_Keepsake_EmptyPool', 'Whitebox.Empty', (0, 650, 0), [])
if 'GiftLane_Instructions' not in existing:
    sign = actors.spawn_actor_from_class(unreal.TextRenderActor, unreal.Vector(150, -700, 210), unreal.Rotator(pitch=0, yaw=180, roll=0))
    sign.set_actor_label('GiftLane_Instructions')
    text = sign.get_component_by_class(unreal.TextRenderComponent)
    text.set_world_size(24)
    text.set_text('GIFT TEST\nE: open nearby box\nRandom / Step Stone / Keepsake\nOne reward per box per run')
    sign.set_actor_hidden_in_game(True)
if 'Instructions' in existing:
    existing['Instructions'].set_actor_hidden_in_game(True)
if 'GiftLane_SafeSave' not in existing:
    safe = actors.spawn_actor_from_class(unreal.TripoZone, unreal.Vector(0, -500, 100))
    safe.set_actor_label('GiftLane_SafeSave')
    safe.set_editor_property('kind', unreal.TripoZoneKind.SAFE)
    safe.set_editor_property('hint', 'SAFE / Save from pause menu')
    safe.volume.set_box_extent(unreal.Vector(250, 650, 160))
    safe.get_component_by_class(unreal.TextRenderComponent).set_visibility(False)
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
triggers = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.TripoChaseTrigger)
if triggers:
    triggers[0].build_navigation_for_level()
assert levels.save_current_level()
print('GIFT_WHITEBOX_READY', MAP, BP)
