import unreal
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
job=unreal.AssetImportTask();job.set_editor_property('filename',str(root/'tools/art/frontend/main-menu-desk.png'));job.set_editor_property('destination_path','/Game/UI/FrontEnd');job.set_editor_property('destination_name','T_MainMenuDesk');job.set_editor_property('automated',True);job.set_editor_property('save',True);job.set_editor_property('replace_existing',True)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([job])
t=unreal.load_asset('/Game/UI/FrontEnd/T_MainMenuDesk');t.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON);t.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI);t.set_editor_property('mip_gen_settings',unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS);unreal.EditorAssetLibrary.save_loaded_asset(t)
print(t.get_path_name())
