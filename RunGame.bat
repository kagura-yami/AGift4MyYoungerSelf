@echo off
REM ============================================================
REM  AGift4MyYoungerSelf - 直接启动独立游戏（不经过编辑器）
REM  独立游戏进程不依赖 DDC/Zen，::1 抽风时也能跑。
REM ============================================================
setlocal
cd /d "%~dp0game\Binaries\Win64"
echo [RunGame] Launching standalone game...
start "" "Tripothon.exe" "%~dp0game\Tripothon.uproject" %*
endlocal
