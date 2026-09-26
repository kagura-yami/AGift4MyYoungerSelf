@echo off
REM ============================================================
<<<<<<< HEAD
REM  Tripothon - 直接启动独立游戏（不经过编辑器）
REM  说明：独立游戏进程不依赖 DDC/Zen，即使 ::1 抽风也能跑。
=======
REM  AGift4MyYoungerSelf - 直接启动独立游戏（不经过编辑器）
REM  独立游戏进程不依赖 DDC/Zen，::1 抽风时也能跑。
>>>>>>> 5ddc9a020fa3d6b6b0a8a0b5b00187acfcf7edb9
REM ============================================================
setlocal
cd /d "%~dp0game\Binaries\Win64"
echo [RunGame] Launching standalone game...
start "" "Tripothon.exe" "%~dp0game\Tripothon.uproject" %*
endlocal
