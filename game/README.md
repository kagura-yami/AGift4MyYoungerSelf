# Tripothon 游戏工程

UE **5.7.4** / Windows / 单机。已接通八项能力、机关、挑战奖励、三级成长、NPC 交换、安全存档、34 个剧情事件、功能 UI 和连续教学地图。

## 启动与构建

在仓库根目录执行：

```powershell
./tools/scripts/build_game.ps1
./tools/scripts/open_editor.ps1
```

编辑器默认打开实验场 `L_LogicLab`；故事地图为 `L_Tutorial`。独立包默认启动故事主菜单。最终包位置与状态见 [开发进度](../docs/开发进度.md)。

```powershell
./tools/scripts/package_game.ps1 -Output artifacts/LogicDevelopment
./tools/scripts/package_game.ps1 -Configuration Shipping -Output artifacts/CameraPitch
```

包的入口为输出目录内 `Windows/Tripothon.exe`。重新编译、Automation 和打包前关闭项目编辑器；同时只启动一个编辑器实例。

## 操作与进度

鼠标左右转向、上下观察（−75° 至 +60°）。WASD 沿视角的水平朝向移动，抬头低头不改变平面速度与角色高度。

WASD 移动、鼠标转向、Space 跳跃/蹬墙、Shift 平面位移、Ctrl 上位移、Q 预览/放置垫脚石、F 减速、R 自身时回、T 机关时回、C 分身/取消、E 交互、Backspace 重开挑战、Esc 菜单。技能逐步解锁，HUD 只显示已获得能力。F9 仅在实验场重置练习位置。

普通死亡不会重置挑战钟。挑战结束后只结算一次；重玩已完成挑战没有额外升级。菜单中的独立练习入口先保存故事，再进入隔离的练习进度。实验场菜单可解锁全部三级能力。

存档在 Unreal 的 `Saved/SaveGames` 下，交替写 Story_A/B；Lab_A/B 单独保存。安全地面才允许保存，未结算挑战不可存。读取会清掉临时能力效果，保留已经提交的进度。

## 验证

```powershell
./tools/scripts/run_automation.ps1 -Project game/Tripothon.uproject -Test Tripothon -Report tools/evaluation/reports/local-automation
```

当前 12 个 Automation 用例，另有 `Content/Python/tripo_*_verify.py` 运行回归。脚本需要已运行的正确地图和角色；`tripo_route_verify.run(report_path)` 要求全新教程，从出生点通过 Enhanced Input 连续行走，无传送。

编辑器 MCP 备用客户端：

```powershell
./tools/scripts/setup_tools.ps1
./tools/.venv/Scripts/python.exe tools/scripts/unreal.py --code 'import unreal; print(unreal.Paths.project_dir())'
```

UE5.7 的 `LevelEditorSubsystem.editor_request_begin_play()` 启动真正的 PIE；`editor_play_simulate()` 是 SIE，不能把自行生成的角色称为真实玩家输入测试。开始/结束均为异步请求，下一次调用须确认实际 World/Pawn。

装配入口：`tripo_lab.build()` 重建基础 Lab，`tripo_world_lab.build()` 更新其机关区，`tripo_tutorial.build()` 重建整张教程。仅用于这些生成地图，不可对人工关卡直接调用。`tripo_assets_verify.run(report_path)` 检查配置与项目引用。

## 接入与边界

详细配置、恢复顺序、存档和测试结果见 [全逻辑实现与验收](../docs/全逻辑实现与验收.md)。运行时不依赖 Python、MCP 或编辑器。显示模型、材质和声音可后续替换，机关 `Visual` 接美术，`Body` 保留玩法碰撞。

美术、原 Minecraft 地图/地标/合影和正式难度定稿尚待团队提供。旧的 `artifacts/Windows`、`artifacts/T03` 是历史基线包，不代表当前完整功能。
