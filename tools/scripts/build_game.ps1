param([string]$Engine='C:/Apps/UE/UE_5.7', [ValidateSet('Editor','Game')][string]$Target='Editor')
$ErrorActionPreference='Stop'
$projectPath=(Resolve-Path "$PSScriptRoot/../../game/Tripothon.uproject").Path
$targetName=if($Target -eq 'Editor'){'TripothonEditor'}else{'Tripothon'}
& "$Engine/Engine/Build/BatchFiles/Build.bat" $targetName Win64 Development "-Project=$projectPath" -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2 -NoUBA
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
