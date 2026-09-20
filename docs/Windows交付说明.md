# Windows 逻辑版交付

2026-09-16。UE 5.7.4，Windows x64，单机，D3D11。交付内容为可玩的逻辑灰盒，非最终美术版本。

## 启动与退出

- 最新视角版：`C:/Dev/Projects/UE/Tripothon/artifacts/CameraPitch/Windows/Tripothon.exe`，鼠标支持左右和上下观察，WASD 仅取视角水平朝向。
- 原逻辑试玩版：`C:/Dev/Projects/UE/Tripothon/artifacts/LogicShipping/Windows/Tripothon.exe`，保留用于历史对照，不含上下视角修正。
- 调试版：`C:/Dev/Projects/UE/Tripothon/artifacts/LogicDevelopment/Windows/Tripothon.exe`
- 双击启动；Esc 打开菜单，选择“退出游戏”。分发时复制整个 `Windows` 文件夹，不能只复制 EXE。
- 两个包均包含 `L_Tutorial` 和 `L_LogicLab`，默认打开故事主菜单。教程按剧情逐步解锁能力；实验场菜单可解锁全部三级能力。

## 构建与验证结果

2026-09-16 上下视角补丁：`CameraPitch` Shipping 构建、Cook、打包全部成功（35.98 秒）。12 项俯仰 PIE 检查、12 项 Automation 通过。证据见 `docs/evidence/2026-09-16-pitch/`。此补丁未重复独立窗口全流程试玩，以下独立启动与路线记录为原逻辑版本记录。

| 项目 | 结果 |
|---|---|
| Development | Build、Cook、Stage、Pak/IoStore、Archive 成功，89.33 秒 |
| Shipping | Build、Cook、Stage、Pak/IoStore、Archive 成功，105.27 秒 |
| Shipping 独立启动 | 已实际启动到中文主菜单，进入新游戏后 HUD 与角色显示正常 |
| PIE 连续路线 | 67.78 秒，34 段剧情、8 个挑战、16 次普通跳跃、0 次恢复 |
| 配置与引用 | 10 个配置资产有效，12 个项目包无缺失项目内引用 |
| 测试边界 | 完整剧情、存读档和技能行为主要由 PIE/Automation 验证；未把主菜单截图称为 Shipping 全流程通关 |

测试地图为教程与实验场，机器为 i7-13700 / RTX 5060 / 64 GB RAM。独立窗口检查使用 1280×720。没有针对最终美术场景测量稳定帧时间，因此不声明最终性能达标或优化收益。正式发布前仍需在固定画质、完整路线下采集帧时间和 GPU 时间，建议以目标机 60 FPS（16.67 ms）为预算，并报告中位数、P95 和采样时长。

**结论：可用于逻辑试玩与后续美术接入；尚不作最终商业/赛事发布验收。**

## 已知限制

- 世界内的文字是开发标记，部分朝向和中文字库不适合正式展示；功能中文说明由 HUD/对话承担。
- Cook 有运行时生成 FText 的本地化标识警告，不影响本次构建；正式本地化应接入稳定文本键或 StringTable。
- 原 Minecraft 地图、模型、动画、声音、实际合影和昵称没有提供，现有位置不代表已复原原图。
- 教程基础路径已可通关，技能组合的最终难度和正式地形仍需关卡定稿。

## 重建

先关闭编辑器，再在仓库根执行：

```powershell
./tools/scripts/build_game.ps1
./tools/scripts/run_automation.ps1 -Project game/Tripothon.uproject -Test Tripothon -Report tools/evaluation/reports/recheck
./tools/scripts/package_game.ps1 -Output artifacts/LogicDevelopment
./tools/scripts/package_game.ps1 -Configuration Shipping -Output artifacts/LogicShipping
```

运行包不依赖本机 UE 编辑器、Python 或 MCP。系统所需的图形驱动和 Unreal 运行库仍需满足。

## 保留的产物

`artifacts/Windows`、`artifacts/T03` 为历史基线；当前两个 Logic 包为新交付。`tools/evaluation` 保留原始测试、候选工具、日志和实验环境，`game/Intermediate`、`game/Saved` 保留构建缓存及测试存档。本轮没有自动删除这些目录；它们不纳入源码提交。当前用户试玩的 Shipping 窗口保持打开。

源码与资源的 SHA-256、验收报告和包入口哈希见 `docs/evidence/2026-09-16-logic/manifest.json`。仓库尚未创建提交或推送远端。
