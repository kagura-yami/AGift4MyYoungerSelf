import unreal
path='/Game/UI/Prototype/M_Echo'
material=unreal.load_asset(path)
if not material:
    material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_Echo','/Game/UI/Prototype',unreal.Material,unreal.MaterialFactoryNew())
    material.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property('two_sided',True)
    material.set_editor_property('used_with_skeletal_mesh',True)
    color=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionVectorParameter,-300,0)
    color.set_editor_property('parameter_name','EchoColor')
    color.set_editor_property('default_value',unreal.LinearColor(.12,.7,1,1))
    opacity=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionScalarParameter,-300,180)
    opacity.set_editor_property('parameter_name','EchoOpacity')
    opacity.set_editor_property('default_value',.45)
    unreal.MaterialEditingLibrary.connect_material_property(color,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
    unreal.MaterialEditingLibrary.recompile_material(material)
    assert unreal.EditorAssetLibrary.save_loaded_asset(material), "Echo material save failed"
assert unreal.EditorAssetLibrary.save_loaded_asset(material), "Echo material save failed"
print(path)
