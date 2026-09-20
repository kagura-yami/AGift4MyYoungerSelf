"""Snapshot T02 acceptance evidence, including a possibly still-running game log."""
import hashlib
import json
import shutil
from pathlib import Path

root = Path(__file__).resolve().parents[2]
reports = root / 'tools/evaluation/reports'
target = root / 'docs/evidence/2026-09-15-t02'
target.mkdir(parents=True,exist_ok=True)
for name in ['t02-runtime.json','t02-identity.json','t02-t01-regression.json']:
    assert json.loads((reports/name).read_text())['ok'] is True,name
summary = json.loads((reports/'t02-final-automation/summary.json').read_text(encoding='utf-8-sig'))
assert summary['succeeded']==5 and summary['failed']==0 and summary['exitCode']==0
assert 'BUILD SUCCESSFUL' in (reports/'t02-package.log').read_text(encoding='utf-8',errors='replace')
log = (reports/'t02-package-runtime.log').read_text(encoding='utf-8',errors='replace')
assert 'Pause reason=0 set=1 active=1' in log and 'Pause reason=0 set=0 active=0' in log
names = ['t02-runtime.json','t02-identity.json','t02-t01-regression.json',
         't02-validated-build.log','t02-package.log','t02-package-runtime.log',
         't02-regression-editor.log','t02-persistence.json',
         't02-package-paused.jpg','t02-package-resumed.jpg',
         't02-final-automation/index.json','t02-final-automation/summary.json']

def record(path):
    with path.open('rb') as stream:
        digest = hashlib.file_digest(stream,'sha256').hexdigest()
    return {'path':path.relative_to(root).as_posix(),'bytes':path.stat().st_size,'sha256':digest}

copied = []
for name in names:
    dest = target / name.replace('/','-')
    shutil.copyfile(reports/name,dest)
    copied.append(record(dest))
sources = list((root/'game/Source').rglob('*')) + list((root/'game/Config').glob('*.ini'))
sources += list((root/'game/Content/Python').glob('*.py'))
sources += [root/'game/Content/Maps/L_LogicLab.umap',root/'artifacts/Windows/Tripothon/Binaries/Win64/Tripothon.exe']
manifest = {'milestone':'T02 baseline acceptance','engine':'5.7.4','reports':copied,
            'source_and_binary':[record(p) for p in sorted(sources) if p.is_file()],
            'runtime_log':'Snapshot while the independent game remains open for the user.',
            'limitations':'Recovery phase ordering only; actual checkpoint recovery, abilities and save loading are future tasks.'}
(target/'manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False),encoding='utf-8')
print('Archived',len(copied),'reports to',target)
