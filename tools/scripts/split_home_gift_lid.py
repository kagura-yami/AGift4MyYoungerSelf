"""Create separate animated gift parts from the original mesh, preserving its UVs."""
import unreal
import shutil
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='L_Home_Vertical_02'
backup=root.parent/'tools/evaluation/home-before-gift-animation.umap'
assert not unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()
if not backup.exists(): shutil.copy2(root/'Content/Maps/HomeVertical/Maps/L_Home_Vertical_02.umap',backup)
source=unreal.load_asset('/Game/TripoModels/gift_box_3d_model_1/gift_box_3d_model_1')
parts=[]
for name,flip in [('SM_HomeGiftBody',False),('SM_HomeGiftLid',True)]:
    path='/Game/Blueprint/Gift/'+name
    mesh=unreal.DynamicMesh()
    mesh,outcome=unreal.GeometryScript_AssetUtils.copy_mesh_from_static_mesh(source,mesh,unreal.GeometryScriptCopyMeshFromAssetOptions(),unreal.GeometryScriptMeshReadLOD())
    assert outcome==unreal.GeometryScriptOutcomePins.SUCCESS
    unreal.GeometryScript_MeshBooleans.apply_mesh_plane_cut(mesh,unreal.Transform(location=unreal.Vector(0,0,33)),unreal.GeometryScriptMeshPlaneCutOptions(fill_holes=True,flip_cut_side=flip))
    asset=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else unreal.EditorAssetLibrary.duplicate_asset(source.get_path_name(),path)
    _,outcome=unreal.GeometryScript_AssetUtils.copy_mesh_to_static_mesh(mesh,asset,unreal.GeometryScriptCopyMeshToAssetOptions(),unreal.GeometryScriptMeshWriteLOD())
    assert outcome==unreal.GeometryScriptOutcomePins.SUCCESS
    unreal.EditorAssetLibrary.save_loaded_asset(asset,False)
    parts.append(asset)
    print(name,asset.get_bounding_box())
for gift in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if not isinstance(gift,unreal.TripoGiftBox) or str(gift.gift_id) not in ['Home.Bedside.Random','Home.LivingRoom.UpDash']: continue
    gift.modify()
    gift.box_mesh.set_static_mesh(parts[0])
    gift.lid_mesh.set_static_mesh(parts[1])
    gift.lid_mesh.set_relative_location(unreal.Vector(),False,True)
    gift.lid_mesh.set_relative_scale3d(unreal.Vector(1,1,1))
    gift.set_editor_property('lid_lift',32)
    gift.set_editor_property('open_seconds',1.0)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
