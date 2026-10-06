param([Parameter(Mandatory=$true)][string]$GameDirectory,[Parameter(Mandatory=$true)][string]$Destination,[switch]$IncludeImage,[string]$OfflineZip)
$ErrorActionPreference='Stop'
$game=(Resolve-Path -LiteralPath $GameDirectory).Path
$destinationPath=[IO.Path]::GetFullPath($Destination)
if(Test-Path -LiteralPath $destinationPath){throw 'Destination must be a new directory to avoid mixing game versions.'}
if(-not (Test-Path "$game/Tripothon.exe") -and -not (Test-Path "$game/Windows/Tripothon.exe")){throw 'GameDirectory must be the packaged output containing Tripothon.exe (or Windows/Tripothon.exe).'}
if($OfflineZip -and -not (Test-Path -LiteralPath $OfflineZip -PathType Leaf)){throw 'OfflineZip not found.'}
New-Item -ItemType Directory $destinationPath | Out-Null
Get-ChildItem $PSScriptRoot -File | Where-Object {$_.Name -notin @('.gitignore','.env','frpc.toml') -and $_.Extension -ne '.tar'} | Copy-Item -Destination $destinationPath
Copy-Item -LiteralPath "$PSScriptRoot/public-turn" -Destination "$destinationPath/public-turn" -Recurse
New-Item -ItemType Directory "$destinationPath/Game" | Out-Null
& robocopy $game "$destinationPath/Game" /E /COPY:DAT /DCOPY:DAT /R:1 /W:1 /NFL /NDL /NJH /NJS
if($LASTEXITCODE -ge 8){throw "Copy failed: robocopy $LASTEXITCODE"}
if($IncludeImage){
    $images=@('tripothon-signalling:ue5.7-4fb38b9')
    if($OfflineZip){$images+='nginx:1.28-alpine'}
    & docker save -o "$destinationPath/signalling-image.tar" @images
    if($LASTEXITCODE -ne 0){throw 'Docker image export failed. Build-Image.ps1 must succeed first.'}
}
if($OfflineZip){
    New-Item -ItemType Directory "$destinationPath/Downloads" | Out-Null
    Copy-Item -LiteralPath $OfflineZip -Destination "$destinationPath/Downloads/Tripothon-Windows.zip"
}
Write-Host "Portable deployment exported to $destinationPath. Run Configure.ps1 on the destination PC."
