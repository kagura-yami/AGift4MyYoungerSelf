"""Archive verified T03 reports and matching runtime source hashes."""
import hashlib
import json
import shutil
from pathlib import Path

root = Path(__file__).resolve().parents[2]
reports = root / 'tools/evaluation/reports'
target = root / 'docs/evidence/2026-09-15-t03'
summary = json.loads((reports / 't03-automation/summary.json').read_text(encoding='utf-8-sig'))
assert summary['succeeded'] == 7 and summary['failed'] == 0
assert 'BUILD SUCCESSFUL' in (reports / 't03-package.log').read_text(errors='replace')
startup = (reports / 't03-package-startup.log').read_text(errors='replace')
assert 'Bringing World /Game/Maps/L_LogicLab' in startup and 'Game engine shut down' in startup
target.mkdir(parents=True, exist_ok=True)
names = ['t03-build.log', 't03-package.log', 't03-package-startup.log',
         't03-automation/index.json', 't03-automation/summary.json']
for name in names:
    shutil.copyfile(reports / name, target / name.replace('/', '-'))

def record(path):
    return {'path': path.relative_to(root).as_posix(), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()}

sources = [p for p in (root / 'game/Source').rglob('*') if p.is_file()]
sources.append(root / 'artifacts/T03/Windows/Tripothon/Binaries/Win64/Tripothon.exe')
(target / 'manifest.json').write_text(json.dumps({
    'scope': 'T03 lifecycle foundation; no active gameplay ability effects implemented',
    'startup': 'Packaged NullRHI load and clean exit, not visual/physical-input testing',
    'sources': [record(p) for p in sorted(sources)],
    'reports': [record(target / name.replace('/', '-')) for name in names]
}, indent=2), encoding='utf-8')
print(target)
