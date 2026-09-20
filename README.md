# A Gift for My Younger Self

基于 Unreal Engine 5.7.4 的单机逻辑灰盒项目。项目目标是实现一套可持续扩展的时间能力、机关、剧情、成长和安全存档框架；最终美术资产不在本仓库范围内。

## 目录

- `game/`：UE 工程、C++ 游戏逻辑、地图、数据资产和验证脚本
- `docs/`：开发方案、架构说明、验收记录、交付说明和历史证据
- `tools/`：构建、打包、Automation 和 Unreal Python 调试脚本
- `.agents/`：本项目使用的 UE 专项协作技能说明

## 环境

- Unreal Engine 5.7.4
- Windows x64
- Visual Studio 2022（包含 C++ 游戏开发工具）
- Git LFS（用于 `.uasset` 和 `.umap`）

## 构建与运行

在仓库根目录执行：

```powershell
./tools/scripts/build_game.ps1
./tools/scripts/open_editor.ps1
```

编辑器默认打开 `L_LogicLab`。故事地图为 `L_Tutorial`。打包前关闭编辑器，并确保同一时间只运行一个编辑器实例：

```powershell
./tools/scripts/package_game.ps1 -Configuration Shipping -Output artifacts/CameraPitch
```

输出入口为 `artifacts/CameraPitch/Windows/Tripothon.exe`；完整 Windows 文件夹需要一起分发。

## 当前逻辑范围

- 视角主导的移动：鼠标左右、上下观察，俯仰范围 −75° 至 +60°；WASD 只使用视角水平朝向
- 八项时间与空间能力、机关图、挑战奖励和三级成长
- 34 个剧情事件、NPC 交换、检查点、安全存档和继续游戏
- 主菜单、暂停、奖励、剧情、收藏和练习入口

## 文档入口

- [开发执行方案](docs/项目开发执行方案.md)
- [项目现状分析](docs/项目现状分析.md)
- [全逻辑实现与验收](docs/全逻辑实现与验收.md)
- [Windows 交付说明](docs/Windows交付说明.md)
- [文档索引](docs/README.md)

## 验证

```powershell
./tools/scripts/run_automation.ps1 -Project game/Tripothon.uproject -Test Tripothon -Report tools/evaluation/reports/automation
```

最近一次视角验证记录位于 `docs/evidence/2026-09-16-pitch/`，包括俯仰输入、水平移动独立性和打包结果。
