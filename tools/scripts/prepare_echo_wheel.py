import unreal
from pathlib import Path
source=str(Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())).parent / 'docs/ui-style/echo-wheel-frame.png')
task=unreal.AssetImportTask()
task.filename=source
task.destination_path='/Game/UI/Prototype'
task.destination_name='T_UI_EchoWheel'
task.automated=True
task.replace_existing=True
task.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture=unreal.load_asset('/Game/UI/Prototype/T_UI_EchoWheel')
assert texture
texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property('srgb',True)
assert unreal.EditorAssetLibrary.save_loaded_asset(texture)
print('Echo wheel texture imported and saved')
