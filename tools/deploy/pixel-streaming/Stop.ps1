$ErrorActionPreference='Stop'
$statePath=Join-Path $PSScriptRoot 'runtime/game-process.json'
if(Test-Path $statePath){
    $state=Get-Content $statePath -Raw | ConvertFrom-Json
    $game=Get-Process -Id $state.pid -ErrorAction SilentlyContinue
    $startedTicks=if($state.startedTicks){[long]$state.startedTicks}else{([datetime]$state.started).ToUniversalTime().Ticks}
    if($game -and $game.Path -eq $state.path -and $game.StartTime.ToUniversalTime().Ticks -eq $startedTicks){
        Stop-Process -Id $game.Id
    }
    Remove-Item -LiteralPath $statePath
}
Push-Location $PSScriptRoot
try { & docker compose --profile downloads down; if($LASTEXITCODE -ne 0){throw 'Docker shutdown failed.'} }
finally { Pop-Location }
Write-Host 'Stopped. Data/ and runtime/ have been preserved.'
