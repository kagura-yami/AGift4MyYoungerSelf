"""Build the reproducible T01 graybox. Call build() only in the lab map/editor."""
import unreal


def build():
    editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    cube = unreal.load_asset('/Engine/BasicShapes/Cube.Cube')

    def block(name, location, scale):
        actor = editor.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location))
        actor.set_actor_label(name)
        actor.static_mesh_component.set_static_mesh(cube)
        actor.set_actor_scale3d(unreal.Vector(*scale))
        return actor

    # Large open practice pad, a collision wall and steps below baseline jump height.
    block('Lab_Floor', (500,0,-30), (32,26,.5))
    block('Lab_CollisionWall', (1100,0,170), (.5,8,3.4))
    block('Lab_Step1', (150,500,25), (2.5,3,.5))
    block('Lab_Step2', (420,500,65), (2.5,3,1.3))
    block('Lab_Step3', (700,500,105), (2.5,3,2.1))
    block('Lab_Landing', (1700,0,10), (4,6,.8))
    start = editor.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0,0,100))
    start.set_actor_label('Lab_PlayerStart')
    region = editor.spawn_actor_from_class(unreal.TripoCameraVolume, unreal.Vector(500,-700,150))
    region.set_actor_label('Lab_Camera90')
    region.set_editor_property('anchor_yaw',90.)
    override = editor.spawn_actor_from_class(unreal.TripoCameraVolume, unreal.Vector(1700,-700,150))
    override.set_actor_label('Lab_CameraPriority')
    override.set_editor_property('anchor_yaw',-30.)
    override.set_editor_property('priority',10)
    light = editor.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0,0,600), unreal.Rotator(-50,-30,0))
    light.light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    light.light_component.set_editor_property('intensity',4.)
    sky = editor.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0,0,400))
    sky.light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    fog = editor.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0,0,-100))
    fog.component.set_editor_property('fog_density',.008)
    assert unreal.EditorLoadingAndSavingUtils.save_map(world, '/Game/Maps/L_LogicLab')
    return {'map':world.get_path_name(),'actors':len(editor.get_all_level_actors())}
