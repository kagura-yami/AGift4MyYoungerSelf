"""Archive verified logic reports and a source/content fingerprint."""
import hashlib
import json
import shutil
from pathlib import Path

root=Path(__file__).resolve().parents[2]
reports=root/'tools/evaluation/reports'
out=root/'docs/evidence/2026-09-16-logic'
out.mkdir(parents=True,exist_ok=True)
checks=['full-logic-assets.json','final-world-runtime.json','final-time-runtime.json',
        'resume-abilities-runtime.json','movement-edges-runtime.json','exchange-runtime.json',
        'story-runtime.json','tutorial-route-runtime.json']
for name in checks:
    data=json.loads((reports/name).read_text(encoding='utf-8-sig'))
    assert data.get('ok') is True, name+' is not a passed report'
    shutil.copy2(reports/name,out/name)
automation=json.loads((reports/'full-logic-automation/index.json').read_text(encoding='utf-8-sig'))
assert automation['succeeded']==12 and all(automation[k]==0 for k in ['failed','notRun','inProcess'])
shutil.copy2(reports/'full-logic-automation/index.json',out/'automation-index.json')
for name in ['story-continue-verified.json','delivery-build.log','logic-development-package.log','logic-shipping-package.log']:
    shutil.copy2(reports/name,out/name)
files=[p for folder in ['game/Source','game/Config','game/Content'] for p in (root/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
files.append(root/'game/Tripothon.uproject')
for name in ['LogicDevelopment','LogicShipping']:
    files.extend(p for p in (root/'artifacts'/name/'Windows').rglob('*.exe') if 'Tripothon' in p.name)
manifest={str(p.relative_to(root)).replace('\\','/'):hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
(out/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf-8')
(out/'desktop-check.json').write_text(json.dumps({
    'shipping_startup':True,'window':'Tripothon','resolution':'1280x720',
    'observed':['Chinese main menu','New-game character and Chinese HUD'],
    'input_note':'User input was detected; automated window input stopped and user continued playing.',
    'full_shipping_traversal_verified':False,
    'final_time_fixture_note':'final-time-runtime metadata retained from initial SIE script; this rerun was launched after editor_request_begin_play on the possessed PIE character.'
},ensure_ascii=False,indent=2),encoding='utf-8')
print(f'Archived {len(manifest)} source/content/executable hashes and {len(list(out.iterdir()))} evidence files')
