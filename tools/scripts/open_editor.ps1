param([string]$Engine='C:/Apps/UE/UE_5.7')
$ErrorActionPreference='Stop'
$projectPath=(Resolve-Path "$PSScriptRoot/../../game/Tripothon.uproject").Path
$arguments='"{0}" -NoLiveCoding -nosplash' -f $projectPath
$editor=Start-Process "$Engine/Engine/Binaries/Win64/UnrealEditor.exe" -ArgumentList $arguments -WindowStyle Hidden -PassThru
Write-Output "Editor PID: $($editor.Id); project: $projectPath"
Write-Output 'MCP becomes ready after editor initialization at http://127.0.0.1:18777/mcp'
