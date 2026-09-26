@echo off
REM ============================================================
REM  Tripothon 编辑器 - 应急启动（跳过 DDC 写入节点要求）
REM  用途：当 ::1 (IPv6 环回) 抽风、ZenServer 连不上、
REM        编辑器报 "no writable nodes available" 崩溃时使用。
REM  代价：本次会话不使用磁盘 DDC，首次加载/着色器编译会变慢。
REM ============================================================
setlocal
set "EDITOR=D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
if not exist "%EDITOR%" (
    echo [RunEditor] Unreal Editor not found: %EDITOR%
    pause
    exit /b 1
)
cd /d "%~dp0"
echo [RunEditor] Launching editor with -DDC-ForceMemoryCache ...
start "" "%EDITOR%" "%~dp0game\Tripothon.uproject" -DDC-ForceMemoryCache -NoLiveCoding -nosplash %*
endlocal
