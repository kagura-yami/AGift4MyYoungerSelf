"""Create the chapter portal's animated, circular water-light material in the editor."""
import unreal
path='/Game/Materials/Portal/M_ChapterPortal'
m=unreal.load_asset(path)
if not m:m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_ChapterPortal','/Game/Materials/Portal',unreal.Material,unreal.MaterialFactoryNew())
e=unreal.MaterialEditingLibrary
e.delete_all_material_expressions(m)
m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_ADDITIVE)
m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
m.set_editor_property('two_sided',True)
uv=e.create_material_expression(m,unreal.MaterialExpressionTextureCoordinate)
t=e.create_material_expression(m,unreal.MaterialExpressionScalarParameter);t.set_editor_property('parameter_name','PortalTime')
c=e.create_material_expression(m,unreal.MaterialExpressionCustom)
pins=[]
for name in ['UV','T']:
    p=unreal.CustomInput();p.set_editor_property('input_name',name);pins.append(p)
c.set_editor_property('inputs',pins);c.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT3)
c.set_editor_property('code','''float2 p=UV-.5; float r=length(p); float a=atan2(p.y,p.x);
float ring=exp(-pow((r-.415)*115,2));
float halo=exp(-pow((r-.415)*28,2))*.35;
float thread=exp(-pow((r-.395-.008*sin(a*9-T*1.7))*180,2))*.6;
float inner=saturate(1-r/.40)*(.10+.04*sin(r*80-a*3+T*2));
return float3(.25,.72,.63)*(ring*3+halo+inner)+float3(1,.72,.30)*thread*2;
''')
e.connect_material_expressions(uv,'',c,'UV');e.connect_material_expressions(t,'',c,'T')
e.connect_material_property(c,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
one=e.create_material_expression(m,unreal.MaterialExpressionConstant);one.set_editor_property('r',1)
e.connect_material_property(one,'',unreal.MaterialProperty.MP_OPACITY)
e.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
print('CHAPTER_PORTAL_MATERIAL_SAVED')
