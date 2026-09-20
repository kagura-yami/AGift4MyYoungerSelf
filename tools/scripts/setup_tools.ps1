$ErrorActionPreference='Stop'
$toolsRoot=[IO.Path]::GetFullPath("$PSScriptRoot/..")
if(-not (Test-Path "$toolsRoot/.venv/Scripts/python.exe")) {
    uv venv "$toolsRoot/.venv" --python 3.12
    if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
}
uv pip install --python "$toolsRoot/.venv/Scripts/python.exe" -r "$toolsRoot/requirements.lock.txt"
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
