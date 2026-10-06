"""Import the supplied card unchanged; cut its four windows in the material."""
import unreal
from pathlib import Path
import shutil

root=Path(unreal.Paths.project_dir()).resolve().parent
source=root/'tools/assets/office_cipher/card.png'
source.parent.mkdir(parents=True,exist_ok=True)
if not source.exists():
    shutil.copy2('C:/Users/KAGURA~1/AppData/Local/Temp/codex-clipboard-c4df5d7a-5a94-468a-a637-3202b151337f.png',source)
path='/Game/UI/OfficeCipher'
texture=unreal.load_asset(path+'/T_CipherCard')
if not texture:
    task=unreal.AssetImportTask()
    for k,v in [('filename',str(source)),('destination_path',path),('destination_name','T_CipherCard'),('automated',True),('save',True)]:task.set_editor_property(k,v)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture=unreal.load_asset(path+'/T_CipherCard')
texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property('never_stream',True)
unreal.EditorAssetLibrary.save_loaded_asset(texture)
e=unreal.MaterialEditingLibrary
for name,ui in [('M_CipherCard_UI',True),('M_CipherCard_World',False)]:
    m=unreal.load_asset(path+'/'+name) or unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,path,unreal.Material,unreal.MaterialFactoryNew())
    e.delete_all_material_expressions(m)
    m.set_editor_property('material_domain',unreal.MaterialDomain.MD_UI if ui else unreal.MaterialDomain.MD_SURFACE)
    m.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT if ui else unreal.BlendMode.BLEND_MASKED)
    m.set_editor_property('two_sided',True)
    sample=e.create_material_expression(m,unreal.MaterialExpressionTextureSample)
    sample.set_editor_property('texture',texture)
    uv=e.create_material_expression(m,unreal.MaterialExpressionTextureCoordinate)
    mask=e.create_material_expression(m,unreal.MaterialExpressionCustom)
    pin=unreal.CustomInput();pin.set_editor_property('input_name','UV')
    mask.set_editor_property('inputs',[pin]);mask.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT1)
    mask.set_editor_property('code','''float2 p=UV*float2(810,1440);
float d=min(min(length(p-float2(169.5,520.5))-62,length(p-float2(197.5,999))-66),min(length(p-float2(547,518))-62.5,length(p-float2(581.5,990))-72.5));
float2 q=abs(p-float2(405,720))-float2(369,684);
float rounded=length(max(q,0))+min(max(q.x,q.y),0)-36;
return smoothstep(-1,1,d)*(1-smoothstep(-1,1,rounded));''')
    e.connect_material_expressions(uv,'',mask,'UV')
    e.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR if ui else unreal.MaterialProperty.MP_BASE_COLOR)
    e.connect_material_property(mask,'',unreal.MaterialProperty.MP_OPACITY if ui else unreal.MaterialProperty.MP_OPACITY_MASK)
    if not ui:
        rough=e.create_material_expression(m,unreal.MaterialExpressionConstant);rough.set_editor_property('r',.85)
        e.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    e.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
print('CIPHER_CARD_MATERIALS_READY')
