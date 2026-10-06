param([ValidateRange(640,3840)][int]$Width=1920,[ValidateRange(360,2160)][int]$Height=1080,[ValidateRange(15,60)][int]$Fps=60)
$ErrorActionPreference='Stop'
$runtime=Join-Path $PSScriptRoot 'runtime'
$statePath=Join-Path $runtime 'game-process.json'
if(-not (Test-Path "$runtime/signalling.json")){throw 'Run Configure.ps1 first.'}
$hostConfig=Get-Content "$runtime/host.json" -Raw | ConvertFrom-Json
$webPort=[int]$hostConfig.webPort
$streamerPort=[int]$hostConfig.streamerPort
$executables=@(
    "$PSScriptRoot/Game/Tripothon/Binaries/Win64/Tripothon-Win64-Shipping.exe",
    "$PSScriptRoot/Game/Tripothon/Binaries/Win64/Tripothon.exe",
    "$PSScriptRoot/Game/Windows/Tripothon/Binaries/Win64/Tripothon-Win64-Shipping.exe",
    "$PSScriptRoot/Game/Windows/Tripothon/Binaries/Win64/Tripothon.exe"
)
$exe=$executables | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if(-not $exe){throw 'Packaged game not found under Game/. Use Export.ps1 on the development PC.'}
$exe=(Resolve-Path -LiteralPath $exe).Path
if(Test-Path $statePath){
    $old=Get-Content $statePath -Raw | ConvertFrom-Json
    $running=Get-Process -Id $old.pid -ErrorAction SilentlyContinue
    $oldTicks=if($old.startedTicks){[long]$old.startedTicks}else{([datetime]$old.started).ToUniversalTime().Ticks}
    if($running -and $running.Path -eq $old.path -and $running.StartTime.ToUniversalTime().Ticks -eq $oldTicks){throw 'This deployment is already running. Use Stop.ps1 first.'}
}
Push-Location $PSScriptRoot
try {
    $composeArgs=@('compose')
    if(Test-Path "$PSScriptRoot/Downloads/Tripothon-Windows.zip"){$composeArgs+=@('--profile','downloads')}
    & docker @composeArgs up -d --wait --wait-timeout 120
    if($LASTEXITCODE -ne 0){throw 'Docker startup failed. Check Docker Desktop (Linux containers) and run Build-Image.ps1.'}
} finally { Pop-Location }
$data=Join-Path $PSScriptRoot 'Data/game'
New-Item -ItemType Directory -Force $data | Out-Null
$gameArgs=@('-RenderOffscreen','-AudioMixer','-Unattended','-ForceRes',"-ResX=$Width","-ResY=$Height",
    "-PixelStreamingConnectionURL=ws://127.0.0.1:$streamerPort",'-PixelStreamingEncoderCodec=H264',
    '-PixelStreamingDecoupleFramerate',
    "-PixelStreamingWebRTCFps=$Fps",'-PixelStreamingWebRTCMinBitrate=2000000','-PixelStreamingWebRTCMaxBitrate=15000000',
    "-UserDir=`"$data`"", "-ExecCmds=`"t.MaxFPS $Fps`"")
$game=Start-Process -FilePath $exe -ArgumentList $gameArgs -WorkingDirectory (Split-Path $exe) -WindowStyle Hidden -PassThru
Start-Sleep -Seconds 3
if($game.HasExited){throw "Game exited with code $($game.ExitCode). Check Data/game/Saved/Logs and NVIDIA drivers."}
@{pid=$game.Id;path=$exe;startedTicks=$game.StartTime.ToUniversalTime().Ticks} | ConvertTo-Json | Set-Content $statePath
Write-Host "Started. Open http://localhost:$webPort/ . FRP should forward local TCP port $webPort only."
Write-Host "Game data: $data. One connected player at a time; progress is shared between visitors."
