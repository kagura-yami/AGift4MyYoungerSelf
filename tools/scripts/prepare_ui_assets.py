"""Validate UI sources outside UE; --import-assets imports review textures in UE Python.

Run without arguments for a read-only preflight. Imported raw art is deliberately
kept in /Game/UI/Prototype, not treated as production-ready transparent widgets.
"""
import argparse
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    'T_UI_SkillIconsAlpha': 'docs/ui-style/skill-icons-transparent.png',
    'T_UI_SkillBook': 'docs/ui-style/skill-book-generated.png',
    'T_UI_SkillIcons': 'docs/ui-style/skill-icons-generated.png',
    'T_UI_HUD_Raw': 'docs/ui-style/assets-v3/hud-raw.png',
    'T_UI_Pause_Raw': 'docs/ui-style/assets-v3/pause-raw.png',
    'T_UI_Upgrade_Raw': 'docs/ui-style/assets-v3/upgrade-raw.png',
    'T_UI_Item_Raw': 'docs/ui-style/assets-v3/item-raw.png',
    'T_UI_Dialogue_Raw': 'docs/ui-style/assets-v3/dialogue-raw.png',
}


def preflight():
    rows = []
    for name, source in SOURCES.items():
        path = ROOT / source
        with path.open('rb') as stream:
            header = stream.read(33)
        if header[:8] != b'\x89PNG\r\n\x1a\n' or header[12:16] != b'IHDR':
            raise ValueError(f'Invalid PNG: {source}')
        width, height = struct.unpack('>II', header[16:24])
        if width == 0 or height == 0:
            raise ValueError(f'Empty texture: {source}')
        rows.append(dict(name=name, source=source, width=width, height=height,
                         alpha_channel=header[25] in (4, 6),
                         destination=f'/Game/UI/Prototype/{name}'))
    return rows


def import_assets():
    import unreal
    rows = preflight()  # Validate every input before mutating the project.
    for row in rows:
        destination = row['destination']
        if unreal.EditorAssetLibrary.does_asset_exist(destination):
            existing = unreal.EditorAssetLibrary.load_asset(destination)
            if not isinstance(existing, unreal.Texture2D):
                raise RuntimeError(f'Existing asset is not a Texture2D: {destination}')
            existing.set_editor_property('lod_group', unreal.TextureGroup.TEXTUREGROUP_UI)
            existing.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_EDITOR_ICON)
            if not unreal.EditorAssetLibrary.save_loaded_asset(existing):
                raise RuntimeError(f'Save failed: {destination}')
            unreal.log(f'UI import: keep existing pixels {destination}')
            continue
        task = unreal.AssetImportTask()
        task.set_editor_property('filename', str(ROOT / row['source']))
        task.set_editor_property('destination_path', '/Game/UI/Prototype')
        task.set_editor_property('destination_name', row['name'])
        task.set_editor_property('automated', True)
        task.set_editor_property('replace_existing', False)
        task.set_editor_property('save', False)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        texture = unreal.EditorAssetLibrary.load_asset(destination)
        if not isinstance(texture, unreal.Texture2D):
            raise RuntimeError(f'Import failed: {destination}')
        texture.set_editor_property('lod_group', unreal.TextureGroup.TEXTUREGROUP_UI)
        texture.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        texture.set_editor_property('srgb', True)
        if not unreal.EditorAssetLibrary.save_loaded_asset(texture):
            raise RuntimeError(f'Save failed: {destination}')
        unreal.log(f'UI import: saved {destination}')
    return rows


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--import-assets', action='store_true')
    args = parser.parse_args()
    print(json.dumps(import_assets() if args.import_assets else preflight(), ensure_ascii=False, indent=2))
