param([string]$Engine='C:/Apps/UE/UE_5.7', [string]$Output="$PSScriptRoot/../../artifacts/Windows", [ValidateSet('Development','Shipping')][string]$Configuration='Development')
$ErrorActionPreference='Stop'
$projectPath=(Resolve-Path "$PSScriptRoot/../../game/Tripothon.uproject").Path
$outputPath=[IO.Path]::GetFullPath($Output)
& "$Engine/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun "-project=$projectPath" -noP4 -platform=Win64 "-clientconfig=$Configuration" -build -cook -stage -pak -iostore -archive "-archivedirectory=$outputPath" '-map=/Game/Maps/L_LogicLab+/Game/Maps/L_Tutorial' -unattended -utf8output '-UbtArgs=-MaxParallelActions=2 -NoUBA'
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
