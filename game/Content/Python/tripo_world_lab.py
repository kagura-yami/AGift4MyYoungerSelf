"""Add the reproducible mechanism/challenge areas to the generated LogicLab."""
import unreal


def build():
    editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    assert world and 'L_LogicLab' in world.get_path_name()
    for actor in editor.get_all_level_actors():
        if actor.get_actor_label().startswith('WorldLab_'):
            editor.destroy_actor(actor)

    def spawn(cls, label, xyz, scale=None):
        a = editor.spawn_actor_from_class(cls, unreal.Vector(*xyz))
        a.set_actor_label('WorldLab_' + label)
        if scale:
            a.set_actor_scale3d(unreal.Vector(*scale))
        return a

    floor = spawn(unreal.StaticMeshActor, 'Floor', (4200, 0, -30), (44, 26, .5))
    floor.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube.Cube'))
    plate = spawn(unreal.TripoMechanism, 'Plate', (2700, 0, 15), (2, 2, .2))
    plate.set_editor_property('allow_echo', True)
    switch = spawn(unreal.TripoMechanism, 'Switch', (2800, 400, 50), (.6, .6, 1))
    switch.set_editor_property('kind', unreal.TripoMechanismKind.SWITCH)
    gate = spawn(unreal.TripoMechanism, 'Gate', (3100, 0, 150), (.3, 6, 3))
    gate.set_editor_property('kind', unreal.TripoMechanismKind.GATE)
    gate.set_editor_property('inputs', [plate, switch])
    gate.set_editor_property('require_all', False)
    platform = spawn(unreal.TripoMechanism, 'Platform', (3500, 500, 80), (3, 3, .3))
    platform.set_editor_property('kind', unreal.TripoMechanismKind.PLATFORM)
    platform.set_editor_property('travel', unreal.Vector(650, 0, 100))

    path = '/Game/Data/DA_LabChallenge'
    definition = unreal.load_asset(path)
    if not definition:
        factory = unreal.DataAssetFactory()
        factory.set_editor_property('data_asset_class', unreal.TripoChallengeDefinition)
        definition = unreal.AssetToolsHelpers.get_asset_tools().create_asset('DA_LabChallenge', '/Game/Data', unreal.TripoChallengeDefinition, factory)
    definition.set_editor_property('challenge_id', 'Lab.FirstRoute')
    definition.set_editor_property('budget', 30.)
    definition.set_editor_property('rewards', [unreal.TripoRewardOption(ability=i, weight=1.) for i in
        (unreal.TripoAbility.DASH, unreal.TripoAbility.UP_DASH, unreal.TripoAbility.WALL_JUMP,
         unreal.TripoAbility.STEP_STONE, unreal.TripoAbility.SLOW, unreal.TripoAbility.REWIND,
         unreal.TripoAbility.ECHO, unreal.TripoAbility.BONUS_TIME)])
    unreal.EditorAssetLibrary.save_loaded_asset(definition)

    def zone(label, kind, xyz, hint):
        a = spawn(unreal.TripoZone, label, xyz)
        a.set_editor_property('kind', kind)
        a.set_editor_property('hint', hint)
        return a

    zone('Checkpoint', unreal.TripoZoneKind.CHECKPOINT, (2200, -500, 0), 'CHECKPOINT')
    start = zone('Start', unreal.TripoZoneKind.START, (2500, -500, 0), 'START / 30 SEC / BACKSPACE RESTART')
    start.set_editor_property('challenge', definition)
    end = zone('Finish', unreal.TripoZoneKind.FINISH, (4100, -500, 0), 'FINISH')
    end.set_editor_property('challenge_id', 'Lab.FirstRoute')
    hazard = zone('Hazard', unreal.TripoZoneKind.HAZARD, (3300, -500, 0), 'HAZARD / JUMP OVER')
    hazard.volume.set_box_extent(unreal.Vector(80, 160, 35))
    build_area = zone('Build', unreal.TripoZoneKind.BUILD_ALLOWED, (4900, 0, 150), 'BUILD AREA')
    build_area.volume.set_box_extent(unreal.Vector(650, 350, 400))
    ceiling = spawn(unreal.StaticMeshActor, 'Ceiling', (-500, 800, 330), (6, 6, .4))
    ceiling.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube.Cube'))
    wall = spawn(unreal.StaticMeshActor, 'JumpWall', (5700, 700, 300), (.5, 6, 6))
    wall.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube.Cube'))
    wall.set_editor_property('tags', ['TripoWallJump'])
    zone('NPC', unreal.TripoZoneKind.NPC, (4600, -500, 0), 'EXCHANGE / SAFE AREA')
    assert unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/Maps/L_LogicLab')
    return {'actors': len(editor.get_all_level_actors()), 'challenge': definition.get_path_name()}
