"""Snapshot view-led control acceptance without overwriting earlier camera evidence."""
import hashlib
import json
import shutil
from pathlib import Path
root=Path(__file__).resolve().parents[2]
reports=root/'tools/evaluation/reports'
target=root/'docs/evidence/2026-09-15-view-driven'
target.mkdir(parents=True,exist_ok=True)
for name in ['view-driven-runtime.json','view-driven-input.json']:
    assert json.loads((reports/name).read_text())['ok'] is True
summary=json.loads((reports/'view-driven-automation/summary.json').read_text(encoding='utf-8-sig'))
assert summary['succeeded']==5 and summary['failed']==0
assert 'BUILD SUCCESSFUL' in (reports/'view-driven-package.log').read_text(encoding='utf-8',errors='replace')
names=['view-driven-runtime.json','view-driven-input.json','view-driven-build.log',
       'view-driven-package.log','view-driven-user-acceptance.json','view-driven-automation/index.json','view-driven-automation/summary.json']
for name in names:
    shutil.copyfile(reports/name,target/name.replace('/','-'))
def record(p):
    with p.open('rb') as file:
        return {'path':p.relative_to(root).as_posix(),'sha256':hashlib.file_digest(file,'sha256').hexdigest()}
sources=list((root/'game/Source').rglob('*'))+list((root/'game/Content/Python').glob('*.py'))
sources += [root/'artifacts/Windows/Tripothon/Binaries/Win64/Tripothon.exe']
(target/'manifest.json').write_text(json.dumps({'contract':'View drives body and movement; body never drives view',
    'sources':[record(p) for p in sorted(sources) if p.is_file()],
    'reports':[record(target/n.replace('/','-')) for n in names]},indent=2))
print(target)
