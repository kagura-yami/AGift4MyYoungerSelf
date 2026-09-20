param(
    [string]$Project = "$PSScriptRoot/../evaluation/Probe/Probe.uproject",
    [string]$Test = 'Tripothon.Probe.Defaults',
    [string]$Report = "$PSScriptRoot/../evaluation/reports/automation-pass",
    [string]$Engine = 'C:/Apps/UE/UE_5.7',
    [int]$TimeoutSeconds = 180
)
$ErrorActionPreference = 'Stop'
$projectPath = (Resolve-Path -LiteralPath $Project).Path
$reportPath = [IO.Path]::GetFullPath($Report)
New-Item -ItemType Directory -Force -Path $reportPath | Out-Null
$logPath = Join-Path $reportPath 'editor.log'
$startedAt = [DateTime]::UtcNow
$arguments = '"{0}" -unattended -nosplash -NoSound -NoLiveCoding -NullRHI -ExecCmds="Automation RunTests {1}" -TestExit="Automation Test Queue Empty" -ReportExportPath="{2}" -abslog="{3}"' -f $projectPath,$Test,$reportPath,$logPath
$process = Start-Process (Join-Path $Engine 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe') -ArgumentList $arguments -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    # This PID was created by this invocation; never terminate another editor.
    Stop-Process -Id $process.Id
    throw "Automation timed out: $Test; inspect $logPath"
}
$process.Refresh()
$indexPath = Join-Path $reportPath 'index.json'
if (-not (Test-Path -LiteralPath $indexPath)) { throw "Missing automation report: $indexPath (exit $($process.ExitCode))" }
if ((Get-Item -LiteralPath $indexPath).LastWriteTimeUtc -lt $startedAt) { throw "Stale automation report: $indexPath" }
$reportData = Get-Content -LiteralPath $indexPath -Raw | ConvertFrom-Json
$summary = [ordered]@{test=$Test;exitCode=$process.ExitCode;succeeded=$reportData.succeeded;failed=$reportData.failed;report=$indexPath}
$summary | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $reportPath 'summary.json')
$summary | ConvertTo-Json
if ($process.ExitCode -ne 0 -or $reportData.failed -gt 0 -or $reportData.notRun -gt 0 -or $reportData.inProcess -gt 0 -or ($reportData.succeeded + $reportData.succeededWithWarnings) -lt 1) { exit 1 }
