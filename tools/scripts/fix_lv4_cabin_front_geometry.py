"""Trim imported moving shell behind landing doors without touching source art."""
import unreal, shutil, time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world() is None
assert ls.get_current_level().get_outer().get_name()=='lv4'
assert ls.save_current_level()
out=root/'tools/evaluation/elevator-rework'
shutil.copy2(root/'game/Content/Maps/lv4.umap',out/('lv4-before-shell-cut-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
source=unreal.load_asset('/Game/Blueprint/Lv4/SM_Lv4MovingCabin')
target_path='/Game/Blueprint/Lv4/SM_Lv4MovingCabin_Clearance'
mesh=unreal.load_asset(target_path) or unreal.EditorAssetLibrary.duplicate_asset(source.get_path_name().split('.')[0],target_path)
dynamic=unreal.DynamicMesh()
_,result=unreal.GeometryScript_AssetUtils.copy_mesh_from_static_mesh(source,dynamic,unreal.GeometryScriptCopyMeshFromAssetOptions(),unreal.GeometryScriptMeshReadLOD())
assert result==unreal.GeometryScriptOutcomePins.SUCCESS
# The imported asset faces +Y. Its front fascia previously extended through
# landing doors; cut along local Y=-40 (~10 cm behind the door's inner face).
frame=unreal.Transform(location=unreal.Vector(0,-40,0),rotation=unreal.Rotator(pitch=0,yaw=0,roll=90))
unreal.GeometryScript_MeshBooleans.apply_mesh_plane_cut(dynamic,frame,unreal.GeometryScriptMeshPlaneCutOptions(fill_holes=False,fill_spans=False))
_,result=unreal.GeometryScript_AssetUtils.copy_mesh_to_static_mesh(dynamic,mesh,unreal.GeometryScriptCopyMeshToAssetOptions(),unreal.GeometryScriptMeshWriteLOD())
assert result==unreal.GeometryScriptOutcomePins.SUCCESS
bounds=mesh.get_bounding_box()
assert bounds.max.y < -39.9 and bounds.min.y < -240, str(bounds)
assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
by={a.get_actor_label():a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
shell=by['SM_Bld_Elevator_01'];shell.modify();shell.static_mesh_component.set_static_mesh(mesh)
assert ls.save_current_level()
print('Cabin front trimmed:',bounds)
