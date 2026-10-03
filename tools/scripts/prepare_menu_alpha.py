"""Run through the UE Python bridge; imports the approved alpha menu textures."""
import unreal
from pathlib import Path
root=Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())).parent
for label in ['Pause','Book','Paper','Upgrade','Item','Dialogue']:
    source=root/'docs/ui-style/assets-v4'/f'{label.lower()}-alpha.png'
    assert source.is_file(),source
    name=f'T_UI_{label}Alpha'
    task=unreal.AssetImportTask()
    task.filename=str(source); task.destination_path='/Game/UI/Prototype'; task.destination_name=name
    task.automated=True; task.replace_existing=True; task.save=True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture=unreal.load_asset('/Game/UI/Prototype/'+name)
    assert texture
    texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property('srgb',True)
    assert unreal.EditorAssetLibrary.save_loaded_asset(texture)
    print('Saved alpha menu:',name)
