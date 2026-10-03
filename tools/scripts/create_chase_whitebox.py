"""Run via unreal.py --file. Creates a separate map; refuses to overwrite existing work."""
import unreal

MAP = '/Game/Maps/L_ChaseWhitebox'
if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    raise RuntimeError('Map already exists; open it instead of overwriting designer changes: ' + MAP)
if unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages():
    raise RuntimeError('Save your current map before creating the chase whitebox.')
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert levels.new_level(MAP)
cube = unreal.load_asset('/Engine/BasicShapes/Cube')

def flat_material(name, color):
    path = '/Game/Materials/Whitebox/' + name
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, '/Game/Materials/Whitebox', unreal.Material, unreal.MaterialFactoryNew())
    value = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant3Vector)
    value.set_editor_property('constant', unreal.LinearColor(*color))
    unreal.MaterialEditingLibrary.connect_material_property(value, '', unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material

hide_material = flat_material('M_ChaseHide', (.035, .45, .12, 1))
trigger_material = flat_material('M_ChaseTrigger', (.8, .2, .025, 1))

def spawn(cls, name, xyz, rot=(0, 0, 0)):
    actor = actors.spawn_actor_from_class(cls, unreal.Vector(*xyz), unreal.Rotator(pitch=rot[0], yaw=rot[1], roll=rot[2]))
    assert actor, name
    actor.set_actor_label(name)
    return actor

def block(name, xyz, size):
    actor = spawn(unreal.StaticMeshActor, name, xyz)
    mesh = actor.static_mesh_component
    mesh.set_static_mesh(cube)
    mesh.set_collision_profile_name('BlockAll')
    actor.set_actor_scale3d(unreal.Vector(*(v / 100 for v in size)))
    return actor

def label(name, xyz, text):
    actor = spawn(unreal.TextRenderActor, name, xyz, (0, 180, 0))
    component = actor.get_component_by_class(unreal.TextRenderComponent)
    component.set_text(text)
    component.set_world_size(38)
    return actor

block('Floor', (1000, 0, -50), (4000, 2800, 100))
block('Wall_North', (1000, 1400, 200), (4000, 60, 400))
block('Wall_South', (1000, -1400, 200), (4000, 60, 400))
block('Wall_West', (-1000, 0, 200), (60, 2800, 400))
block('Wall_East', (3000, 0, 200), (60, 2800, 400))
block('Obstacle_RouteAround', (1100, 0, 180), (100, 1000, 360))
block('Hide_BackWall', (2100, 750, 180), (100, 650, 360))
spawn(unreal.PlayerStart, 'Start_Retry', (0, 0, 100))
trigger = spawn(unreal.TripoChaseTrigger, 'Chase_EnterToStart', (400, 0, 110))
trigger.volume.set_box_extent(unreal.Vector(110, 450, 130))
block('Trigger_FloorMarker', (400, 0, 1), (200, 880, 2)).static_mesh_component.set_material(0, trigger_material)
point = spawn(unreal.TargetPoint, 'Chaser_Spawn', (-650, 0, 100))
trigger.set_editor_property('spawn_point', point)
hide = spawn(unreal.TripoChaseHideZone, 'Hide_AggroDecay', (2330, 750, 120))
hide.label.set_relative_rotation(unreal.Rotator(pitch=0, yaw=180, roll=0), False, True)
hide.volume.set_box_extent(unreal.Vector(180, 280, 140))
block('Hide_FloorMarker', (2330, 750, 2), (350, 550, 4)).static_mesh_component.set_material(0, hide_material)
instant = spawn(unreal.TripoChaseHideZone, 'Hide_InstantClear', (2400, -950, 120))
instant.label.set_relative_rotation(unreal.Rotator(pitch=0, yaw=180, roll=0), False, True)
instant.set_editor_property('aggro_reduction_per_second', 0)
block('InstantHide_FloorMarker', (2400, -950, 2), (300, 300, 4)).static_mesh_component.set_material(0, hide_material)
label('Instructions', (500, -600, 220), 'CHASE WHITEBOX\nCross X=400 to start\nP: pause | Jump / Dash: escape\nHIDE areas: lose aggro')
label('DecayHint', (2500, 750, 400), 'HIDE: 45 aggro / second')
label('InstantHint', (2600, -950, 400), 'HIDE: instant clear')
light = spawn(unreal.DirectionalLight, 'Sun', (0, 0, 1200), (-55, -35, 0))
light.light_component.set_editor_property('intensity', 4)
light.light_component.set_editor_property('forward_shading_priority', 1)
spawn(unreal.SkyLight, 'SkyLight', (1000, 0, 1000))
fill = spawn(unreal.DirectionalLight, 'Fill', (0, 0, 1100), (-60, 150, 0))
fill.light_component.set_editor_property('intensity', 1.5)
fill.light_component.set_editor_property('cast_shadows', False)
nav = spawn(unreal.NavMeshBoundsVolume, 'Navigation_ChaseArena', (1000, 0, 250))
nav.set_actor_scale3d(unreal.Vector(21, 15, 5))
print('NAV_BOUNDS', nav.get_actor_bounds(False))
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
trigger.build_navigation_for_level()
assert levels.save_current_level()
print('CHASE_MAP_CREATED', MAP)
