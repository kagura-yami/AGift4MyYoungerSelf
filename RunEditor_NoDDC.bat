@echo off
setlocal
REM Bypass Zen with a filesystem cache; memory cache remains a fallback.
set "EDITOR=D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
if not exist "%EDITOR%" (
    echo [RunEditor] Unreal Editor not found: %EDITOR%
    pause
    exit /b 1
)
if not exist "%~dp0game\Tripothon.uproject" (
    echo [RunEditor] Project file not found.
    pause
    exit /b 1
)
cd /d "%~dp0"
echo [RunEditor] Launching editor. Initial shader compilation may take several minutes.
echo [RunEditor] Please wait for the editor; do not launch another copy.
start "" "%EDITOR%" "%~dp0game\Tripothon.uproject" -ddc=InstalledNoZenLocalFallback -DDC-ForceMemoryCache -NoLiveCoding -log %*
endlocal
