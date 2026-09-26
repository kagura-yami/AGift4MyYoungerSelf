@echo off
REM ============================================================
REM  Tripothon - 直接启动独立游戏（不经过编辑器）
REM  说明：独立游戏进程不依赖 DDC/Zen，即使 ::1 抽风也能跑。
REM ============================================================
setlocal
cd /d "%~dp0game\Binaries\Win64"
echo [RunGame] Launching standalone game...
start "" "Tripothon.exe" "%~dp0game\Tripothon.uproject" %*
endlocal
