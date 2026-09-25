@echo off
REM ============================================================
REM  AGift4MyYoungerSelf 编辑器 - 应急启动（跳过 DDC 写入节点要求）
REM  用途：::1 (IPv6 环回) 抽风、ZenServer 连不上、
REM        编辑器报 "no writable nodes available" 崩溃时使用。
REM  代价：本次会话不使用磁盘 DDC，首次加载/着色器编译会变慢。
REM ============================================================
setlocal
cd /d "%~dp0game\Binaries\Win64"
echo [RunEditor] Launching editor with -DDC-ForceMemoryCache ...
start "" "UnrealEditor.exe" "%~dp0game\Tripothon.uproject" -DDC-ForceMemoryCache -NoLiveCoding -nosplash
endlocal
