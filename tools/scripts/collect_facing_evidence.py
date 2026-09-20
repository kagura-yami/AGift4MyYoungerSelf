import hashlib
import json
import shutil
from pathlib import Path

root = Path(__file__).resolve().parents[2]
reports = root/'tools/evaluation/reports'
target = root/'docs/evidence/2026-09-15-facing'
target.mkdir(parents=True,exist_ok=True)
for name in ['facing-runtime.json','facing-input.json']:
    assert json.loads((reports/name).read_text())['ok'] is True
assert 'BUILD SUCCESSFUL' in (reports/'facing-package.log').read_text(encoding='utf-8',errors='replace')
summary = json.loads((reports/'facing-automation/summary.json').read_text(encoding='utf-8-sig'))
assert summary['succeeded']==5 and summary['failed']==0
names = ['facing-runtime.json','facing-input.json','facing-build.log','facing-package.log',
         'facing-package.jpg','facing-automation/index.json','facing-automation/summary.json']
for name in names:
    shutil.copyfile(reports/name,target/name.replace('/','-'))
sources = list((root/'game/Source').rglob('*'))+list((root/'game/Content/Python').glob('*.py'))
sources += [root/'artifacts/Windows/Tripothon/Binaries/Win64/Tripothon.exe']
def record(path):
    with path.open('rb') as file:
        return {'path':path.relative_to(root).as_posix(),'sha256':hashlib.file_digest(file,'sha256').hexdigest()}
(target/'manifest.json').write_text(json.dumps({'contract':'Character-relative camera +/-35 degrees; world regions ignored',
    'sources':[record(p) for p in sorted(sources) if p.is_file()],
    'reports':[record(target/n.replace('/','-')) for n in names]},indent=2),encoding='utf-8')
print(target)
