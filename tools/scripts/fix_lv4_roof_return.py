import unreal, shutil
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='lv4'
shutil.copy2(root/'game/Content/Maps/lv4.umap',root/'tools/evaluation/lv4-before-roof-return.umap')
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
by={a.get_actor_label():a for a in aa.get_all_level_actors()}
p=by['LV4_UpperDoorDashExit'];p.modify();p.set_editor_property('entry_enabled',True)
p.set_actor_rotation(unreal.Rotator(pitch=0,yaw=0,roll=0),False)
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
mat=unreal.load_asset('/Game/Materials/Whitebox/M_Lv4ElevatorMetal')
specs=[('LV4_UpperShaftInnerLeft',(-1605,420,714),(.35,.94,4.3)),('LV4_UpperShaftInnerRight',(-1605,710,714),(.35,.94,4.3)),('LV4_UpperShaftSill',(-1590,565,507),(.5,3.54,.32)),('LV4_UpperShaftHeader',(-1605,565,899),(.22,3.54,.3))]
for name,pos,scale in specs:
    a=by.get(name) or aa.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos))
    a.modify();a.set_actor_label(name);a.set_folder_path('LV4_Mechanisms/WestElevator/FixedLandings')
    a.set_actor_scale3d(unreal.Vector(*scale))
    a.static_mesh_component.set_static_mesh(cube);a.static_mesh_component.set_material(0,mat)
    a.static_mesh_component.set_collision_profile_name('BlockAll')
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_begin_play()
