"""Archive this milestone's reports and hashes without changing earlier evidence."""
import hashlib
import json
import shutil
from pathlib import Path

root = Path(__file__).resolve().parents[2]
reports = root / 'tools/evaluation/reports'
target = root / 'docs/evidence/2026-09-15-t01'
target.mkdir(parents=True, exist_ok=True)

for name in ['t01-input-regions.json', 't01-airborne-final.json']:
    assert json.loads((reports/name).read_text())['ok'] is True, name
summary = json.loads((reports/'t01-automation/summary.json').read_text(encoding='utf-8-sig'))
assert summary['succeeded'] == 2 and summary['failed'] == 0 and summary['exitCode'] == 0
assert 'BUILD SUCCESSFUL' in (reports/'t01-package.log').read_text(encoding='utf-8',errors='replace')
for mode in ['720','1080']:
    log = (reports/f't01-package-{mode}.log').read_text(encoding='utf-8',errors='replace')
    assert 'LogExit: Exiting.' in log, 'Clean shutdown required'
    assert f'systemresolution.resy="{mode}"' in log

names = ['t01-input-regions.json','t01-airborne-final.json','t01-fix-build.log',
         't01-package.log','t01-fixed-editor.log','t01-input-editor.log',
         't01-key-editor.log','t01-package-720.log','t01-package-1080.log',
         't01-os-space.jpg','t01-package-720-jump.jpg','t01-package-720-paused.jpg',
         't01-package-720-resumed.jpg','t01-package-1080.jpg','t01-package-1080-paused.jpg']
entries = []
for name in names:
    source = reports/name
    shutil.copy2(source,target/source.name)
    entries.append(source)
for name in ['index.json','summary.json']:
    source = reports/'t01-automation'/name
    shutil.copy2(source,target/('automation-'+name))
    entries.append(source)
sources = list((root/'game/Source').rglob('*')) + list((root/'game/Config').glob('*.ini'))
sources += list((root/'game/Content/Python').glob('*.py'))
sources += [root/'game/Content/Maps/L_LogicLab.umap',
            root/'artifacts/Windows/Tripothon/Binaries/Win64/Tripothon.exe']

def record(path):
    return {'path':path.relative_to(root).as_posix(),'bytes':path.stat().st_size,
            'sha256':hashlib.file_digest(path.open('rb'),'sha256').hexdigest()}

manifest = {'milestone':'T01 baseline acceptance','engine':'5.7.4',
            'reports':[record(p) for p in entries],
            'source_and_binary':[record(p) for p in sorted(sources) if p.is_file()],
            'manual_checks':'Computer Use: OS Space/W/Escape, 720p windowed and 1080p fullscreen, HUD, pause/resume, graceful exit.',
            'limitations':'Graybox only. Full abilities/challenges/save/story are not implemented.'}
(target/'manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False),encoding='utf-8')
print(f'Archived {len(entries)} reports to {target}')
