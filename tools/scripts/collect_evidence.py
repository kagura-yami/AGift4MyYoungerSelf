"""Keep compact evidence in docs; leave raw experiment outputs in the ignored sandbox."""
import csv
import difflib
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

root = Path(__file__).resolve().parents[2]
reports = root / 'tools/evaluation/reports'
out = root / 'docs/evidence/2026-09-15'
out.mkdir(parents=True, exist_ok=True)
for name in ['versions.json','avatar-health.json','avatar-expected-failure.json',
             'avatar-persisted-marker.json','remi-expected-failure.json','ronildo-error-allowed.json',
             'chir-unconnected.json','game-camera-airborne.json','game-collision-jump-pass.json',
             'api-offline-header.json','api-jump-local.json','skills-metadata.json']:
    shutil.copy2(reports/name, out/name)
for directory in ['automation-pass','automation-fail','game-automation-final']:
    shutil.copy2(reports/directory/'summary.json', out/f'{directory}-summary.json')
for name in ['build-avatar.log','build-remi-final.log','build-ronildo.log','build-chir.log',
             'build-chir-patched.log','build-game-editor-final.log','package-game.log',
             'gauntlet-retry.log','data-validation-console.log','insights.log',
             'skills-unrealxu-validation.log','skills-unrealxu-anchors.log','skills-kevin-citations.log']:
    lines = (reports/name).read_text(encoding='utf-8-sig',errors='replace').splitlines()
    selected = [line for line in lines if any(key in line for key in ['error C2026','Error:','Error_','Exported','exported'])]
    (out/(name+'.excerpt.txt')).write_text('\n'.join(selected+lines[-25:])+'\n',encoding='utf-8')
shutil.copy2(root/'game/Saved/Screenshots/WindowsEditor/ScreenShot00000.png',out/'pie-collision.png')
with (reports/'game-timers.csv').open(encoding='utf-8-sig') as stream:
    timers = [row for row in csv.DictReader(stream) if row['Name']=='TripoCharacterTick']
(out/'trace-summary.json').write_text(json.dumps({'trace_bytes':(reports/'game-runtime.utrace').stat().st_size,'timers':timers},indent=2),encoding='utf-8')
sources = [p for directory in ['Source','Config'] for p in (root/'game'/directory).rglob('*') if p.is_file()]
sources += [root/'game/Tripothon.uproject',root/'game/Content/Maps/L_LogicLab.umap']
manifest = {str(p.relative_to(root)).replace('\\','/'):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
(out/'game-source-sha256.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
patches = root/'tools/patches'
for repo,filename in [('ronildo','ronildo-sdk.patch'),('chir','chir-module-detection.patch')]:
    diff = subprocess.check_output(['git','-C',str(root/'tools/evaluation/repos'/repo),'diff'],text=True,encoding='utf-8')
    (patches/filename).write_text(diff,encoding='utf-8')
original = root/'tools/evaluation/repos/remi/plugin/Source/MCPUnreal/MCPUnreal.Build.cs'
changed = root/'tools/evaluation/Probe/Plugins/MCPUnreal/Source/MCPUnreal/MCPUnreal.Build.cs'
diff = difflib.unified_diff(original.read_text(encoding='utf-8').splitlines(True),changed.read_text(encoding='utf-8').splitlines(True),fromfile='a/plugin/Source/MCPUnreal/MCPUnreal.Build.cs',tofile='b/plugin/Source/MCPUnreal/MCPUnreal.Build.cs')
(patches/'remi-fab.patch').write_text(''.join(diff),encoding='utf-8')
print('Evidence files:',len(list(out.iterdir())))
