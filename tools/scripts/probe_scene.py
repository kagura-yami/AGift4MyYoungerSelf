"""Runs inside Unreal Editor. Creates only /Game/Probe evaluation assets."""
import json
from pathlib import Path
import unreal

def build():
    editor=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    assets=unreal.AssetToolsHelpers.get_asset_tools()
    world=unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    cube=unreal.load_asset('/Engine/BasicShapes/Cube')
    for name,loc,scale in [
        ('Probe_Floor',(300,0,-30),(30,18,0.5)),
        ('Probe_Wall',(500,0,150),(0.5,8,3)),
        ('Probe_Step',(100,400,30),(2,2,0.6)),
        ('Probe_Landmark',(650,-500,200),(2,2,4)),
    ]:
        actor=editor.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*loc))
        actor.set_actor_label(name)
        mesh=actor.static_mesh_component
        mesh.set_static_mesh(cube)
        mesh.set_mobility(unreal.ComponentMobility.STATIC)
        actor.set_actor_scale3d(unreal.Vector(*scale))
    start=editor.spawn_actor_from_class(unreal.PlayerStart,unreal.Vector(0,0,100))
    start.set_actor_label('Probe_Start')
    sun=editor.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,700),unreal.Rotator(-50,-30,0))
    sun.light_component.set_editor_property('intensity',4.0)
    editor.spawn_actor_from_class(unreal.SkyLight,unreal.Vector(0,0,500))
    factory=unreal.BlueprintFactory()
    factory.set_editor_property('parent_class',unreal.Actor)
    bp=unreal.load_asset('/Game/Probe/BP_ProbeMarker')
    if not bp: bp=assets.create_asset('BP_ProbeMarker','/Game/Probe',unreal.Blueprint,factory)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_loaded_asset(bp)
    actor=editor.spawn_actor_from_class(unreal.EditorAssetLibrary.load_blueprint_class('/Game/Probe/BP_ProbeMarker'),unreal.Vector(0,600,100))
    actor.set_actor_label('Probe_PersistedMarker')
    actor.set_editor_property('tags',['Probe.Stable.Marker'])
    assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/Probe/L_Probe')
    report={'map':world.get_path_name(),'actors':len(editor.get_all_level_actors()),'blueprint':bp.get_path_name()}
    root=Path(unreal.Paths.project_dir()).resolve().parent/'reports'
    root.mkdir(exist_ok=True)
    (root/'scene-built.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    unreal.log('PROBE_SCENE_READY '+json.dumps(report))

build()
