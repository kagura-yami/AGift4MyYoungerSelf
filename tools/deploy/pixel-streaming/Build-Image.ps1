param([string]$NodeImage='node:22-bookworm')
$ErrorActionPreference='Stop'
Push-Location $PSScriptRoot
try { & docker compose build --build-arg "NODE_IMAGE=$NodeImage"; if($LASTEXITCODE -ne 0){throw 'Signalling image build failed.'} }
finally { Pop-Location }
