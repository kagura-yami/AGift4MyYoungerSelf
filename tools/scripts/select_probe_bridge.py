import json,sys
from pathlib import Path
root=Path(__file__).resolve().parents[2]/'tools/evaluation/Probe'
path=root/'Probe.uproject'
data=json.loads(path.read_text(encoding='utf-8'))
names={'remi':'MCPUnreal','avatar':'ModelContextProtocol','ronildo':'UnrealMCP','chir':'McpAutomationBridge','none':''}
selected=names[sys.argv[1]]
data['Plugins']=[{'Name':name,'Enabled':True} for name in ['EnhancedInput','PythonScriptPlugin','EditorScriptingUtilities','FunctionalTestingEditor','RemoteControl']]
for name in names.values():
    if name and (root/'Plugins'/name).exists(): data['Plugins'].append({'Name':name,'Enabled':name==selected})
path.write_text(json.dumps(data,indent=2),encoding='utf-8')
print('Selected',selected or 'native-only')
