import unreal,shutil,time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
backup=root/'tools/evaluation/focus-backup';backup.mkdir(exist_ok=True)
shutil.copy2(root/'game/Content/Materials/Whitebox/M_InteractionFocus.uasset',backup/('M_InteractionFocus-'+time.strftime('%Y%m%d-%H%M%S')+'.uasset'))
mat=unreal.load_asset('/Game/Materials/Whitebox/M_InteractionFocus');mel=unreal.MaterialEditingLibrary
mat.modify()
mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
color=unreal.find_object(mat,'MaterialExpressionConstant3Vector_0') or mel.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector)
color.set_editor_property('constant',unreal.LinearColor(.26,.65,.95,1))
mel.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
alpha=unreal.find_object(mat,'MaterialExpressionConstant_0') or mel.create_material_expression(mat,unreal.MaterialExpressionConstant)
alpha.set_editor_property('r',.24)
mel.connect_material_property(alpha,'',unreal.MaterialProperty.MP_OPACITY)
mel.recompile_material(mat);assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
print('Saved pale blue overlay, opacity 0.24')
