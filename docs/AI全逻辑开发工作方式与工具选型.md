# AI 全逻辑开发工作方式与工具选型

调研/实测日期：2026-09-15。Codex 负责全部游戏逻辑，美术制作不在其职责内。本文保留方案与候选比较；**最终实测选中 AvatarGanymede 原生 HTTP MCP，已安装四个项目 Skills，并开始正式工程**。最新证据见 [工具实测报告](./工具实测报告.md)，开发状态见 [开发进度](./开发进度.md)。

## 1. 推荐结论

采用 **C++ 规则实现 + 数据驱动配置 + 可重复的编辑器装配脚本 + 一个编辑器 MCP + 自动化玩法测试 + 独立 Windows 包验证**。

核心投入是让我能检查真实状态、控制角色、复现问题和生成验证场景。Skills 提供规程，MCP 提供操作通道，构建与测试提供实际证据。[Skills 官方说明](https://learn.chatgpt.com/docs/build-skills)、[MCP 官方说明](https://learn.chatgpt.com/docs/extend/mcp?surface=cli)。

| 用途 | 推荐工具 | 接入时间 |
|---|---|---|
| 实现与 API 核查 | UE5.7.4 C++、本机引擎头文件、rg | 开发基线 |
| 构建与打包 | UBT、Build.bat、RunUAT、现有 VS 工具链 | 工程接入后首先验证 |
| 工作流 | UnrealXu 精选 Skills + 项目脚本与任务账本 | 四项已安装并校验 |
| 编辑器操作与观察 | AvatarGanymede/ue5.7-mcp 0.6.1 | 已通过编译和关键场景实测 |
| 灰盒生成 | 编辑器 Python、必要的小型 C++ Editor 辅助 | 第一个玩法闭环前 |
| 规则/场景验证 | Automation Tests、Functional Tests、Data Validation | 每个功能同步交付 |
| 角色操作 | 项目专用输入测试驱动、状态快照与截图 | 尽早建设 |
| 包级回归 | UAT + Gauntlet | 基础包可用后 |
| 版本与性能 | Git/Git LFS、Unreal Insights、CSV/日志 | 基线及阶段交付 |

上表是当前主工作流。下文的首试/备选理由保留调研背景，是否通过以实测报告为准。

## 2. 我的完整交付范围

- 角色、有限镜头、输入、八项能力及三级成长。
- 机关、挑战计时、自选/随机奖励、NPC 交换。
- 时间历史、分身、死亡/检查点/重开、安全存档、转场与去重。
- 剧情事件、教学授予、收藏、结尾驻足和练习隔离。
- 功能性 HUD、菜单、按钮、提示及可替换样式/资源接口。
- LogicLab、连续灰盒教学、章节流程接线与最低配置验证。
- 构建包、测试驱动、回归证据和给关卡/美术的组件使用说明。

最终模型、贴图、配音、音乐、正式特效与美术定稿由外部提供。原 Minecraft 地图资料和真实共同经历也需团队提供；我可以先做明确标注的占位地标。

**全部逻辑完成的结果是可以从新游戏玩到结尾的灰盒包。** 正式美术尚未到位不影响逻辑验收；正式地形的审美布局与赛事材料不计入本轮逻辑职责。主观手感由你试玩反馈，我负责规则、流程与可重复验证。

## 3. 本机核查结果

| 项目 | 已核查状态 |
|---|---|
| UE | `C:\Apps\UE\UE_5.7`，Build.version 为 **5.7.4** |
| 引擎入口 | UnrealEditor.exe、Build.bat、RunUAT.bat 均存在 |
| 源码参考 | CharacterMovementComponent、Enhanced Input 头文件可读 |
| VS | `C:\Apps\VisualStudio`，检测到 C++ 组件和 MSVC 14.38/14.44 目录 |
| Windows SDK | 检测到 10.0.26100.0 头文件目录 |
| 命令工具 | git、rg、Python、uv、Go、CMake、clangd 可发现；Go 1.26.2、uv 0.10.9、Git LFS 3.7.1 可运行 |
| UE 插件文件 | PythonScriptPlugin、PythonAutomationTest、FunctionalTestingEditor、RemoteControl 存在；目标工程是否启用未知 |
| 当前目录 | 已有 game/Tripothon.uproject、Git/LFS、tools、项目 Skills 与 MCP 配置 |
| 同级目录 | 有“我的项目”和 UI 的 .uproject，尚未认定其中任何一个属于本游戏 |
| UE MCP | 本机原生 HTTP 服务已实测；项目 MCP 配置已写入，当前会话使用标准客户端直接调用 |

正式工程已由 UE5.7.4 构建通过，实际选用 MSVC 14.44.35224 与 SDK 10.0.26100.0，Windows Development 包已独立启动。

## 4. Skills：主工作流加少量专项知识

### 4.1 首选 UnrealXu/UnrealEngine5-Skills

首批取 `ue5-cpp-gameplay`、`ue5-architecture`、`ue5-debug-validation`、`ue5-performance-packaging`；对应任务再取 `ue5-world-interaction`、`ue5-save-load-replication`、`ue5-ui-umg-slate`、`ue5-blueprint-workflow`。

选择理由：其内容围绕交付步骤、失败定位和验证组织，适合逐项完成功能。已读取仓库与 C++/调试样本。作者当前说明覆盖 5.6–5.8，仍处于迭代测试阶段；5.8 特有内容必须按本机版本过滤。[仓库](https://github.com/UnrealXu/UnrealEngine5-Skills)、[调试 Skill](https://github.com/UnrealXu/UnrealEngine5-Skills/blob/main/skills/ue5-debug-validation/SKILL.md)。

默认不加载自动路由、PCG 和联网扩展；存档条目只取单机部分。所选 Skill 的 references/scripts 和许可证一起保留，不只复制 SKILL.md。

### 4.2 专项备选

| 来源 | 优点 | 本项目取舍 |
|---|---|---|
| [quodsoler/unreal-engine-skills](https://github.com/quodsoler/unreal-engine-skills) | 角色移动、数据资产、存档、测试等细分 C++ 知识 | 按需查专项条目；已抽读测试 Skill，示例仍需本机编译 |
| [kevinpbuckley/unreal-engine-skills](https://github.com/kevinpbuckley/unreal-engine-skills) | 引擎源码导向，编辑器脚本和自动化参考 | 已读 Python Skill 元数据为 5.8，不能按搜索摘要的 5.7 全盘采用 |
| [ibrews/ue5-mcp](https://github.com/ibrews/ue5-mcp) | 编辑器自动化失败模式、异步与读回经验 | 这是 Skill 手册，不是 MCP 服务；遇到相关问题再参考 |

版本差异样本：[Python Skill 源文](https://github.com/kevinpbuckley/unreal-engine-skills/blob/master/skills/core/ue-editor-scripting-and-python/SKILL.md)。不全量安装多套重叠规程，也不把作者“已验证 API”声明当作本机验证。

### 4.3 应补齐的四个项目规程

原资料列出的 gift-* 实体当前缺失，可按本项目重新编写：

- `gift-repo-orient`：真实工程、版本、模块、构建入口、已有实现与任务证据。
- `gift-gameplay-slice`：一个完整功能的实现、配置、灰盒、失败/恢复和验证。
- `gift-temporal-regression`：时间域、历史代次、分身权限、奖励账本和清理。
- `gift-editor-handoff`：创建资产后编译、保存、读回、PIE 和关卡可配置性验收。

构建/测试/打包入口已封装为 tools/scripts 下的脚本；安装了四项上游 Skill。此处 gift-* 项目专项 Skill 尚未创建，当前用任务账本与工程 README 承载项目规程；未修改 AGENTS.md。

## 5. MCP：一个写入通道，通过实测定版

### 首试候选 remiphilippe/mcp-unreal（调研历史）

其文档列出 UE5.7、Codex 配置、Python 执行、蓝图/Actor 操作、日志、PIE、视口截图（含 PIE UI 选项）和状态查询。服务端为 Go，编辑器能力依赖 MCPUnreal，部分属性接口另依赖 Remote Control。[仓库](https://github.com/remiphilippe/mcp-unreal)、[插件说明](https://github.com/remiphilippe/mcp-unreal/blob/main/plugin/README.md)。

初选理由：脚本执行、运行状态、带 UI 截图与本项目需要吻合，本机已有 Go。实测后发现 Fab 可选依赖误判和 Python 异常仍返回成功，因此最终改选 AvatarGanymede。构建和打包仍直接用终端；玩家传送/转向不能代替连续输入验证。

### 备选比较

| 候选 | 作者文档所述特点 | 何时考虑 |
|---|---|---|
| [AvatarGanymede/ue5.7-mcp](https://github.com/AvatarGanymede/ue5.7-mcp) | Win64 编辑器内 HTTP MCP，单工具、健康检查、发现、批执行/异步任务，无独立服务运行时 | 首选链路过重或不稳定时；另验截图、PIE 和项目状态闭环 |
| [RonildoBraga/unreal-mcp](https://github.com/RonildoBraga/unreal-mcp) | UE5.7+，Python/uv 桥接，反射、截图、PIE、UMG，附 smoke_dispatch | 需要更丰富反射和 UI 操作时 |
| [ChiR24/Unreal_mcp](https://github.com/ChiR24/Unreal_mcp) | TypeScript/C++ 桥接，覆盖范围广、关联插件较多 | 前述工具缺关键能力时再评估 |
| [Codeturion/unreal-api-mcp](https://github.com/Codeturion/unreal-api-mcp) | 按版本查询 API/头文件/弃用信息；补丁版可能回退到主次版本库 | 可作为第二个只读 MCP；本机源码搜索够用时不必增加 |

只保留一个编辑器写入通道，只读 API MCP 可以共存。第三方插件固定 commit/release，不能因 README 的工具数量就判定更可靠。

### 官方 MCP 与回移植

Epic 当前官方 MCP 文档面向 5.8，标注 Experimental，说明功能与 API 尚可能变化。本机 5.7.4 扫描未发现 MCP 插件；本项目不为工具升级引擎。[Epic 官方文档](https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor)。

也查到 [thecodebrozilla/UE_MCP](https://github.com/thecodebrozilla/UE_MCP) 的 5.7 回移植方案，说明涉及多插件及部分实验/NoRedist 标记。当前先选边界较小的桥接，避免增加回移植维护范围。

### 准入验收

在专用测试工程或明确的开发资产目录完成：

1. 读到正确引擎版本、项目和地图。
2. 创建 Actor、真实蓝图/数据资产，修改属性。
3. 编译、保存、重开并读回，属性与 ID 一致。
4. 启动 PIE，等待真实运行态并查询 Pawn/World。
5. 执行普通输入，观察位移、碰撞和机关变化。
6. 获取包含 HUD 的游戏截图；开关菜单后验证输入恢复。
7. 分别执行预期成功和故意失败的测试，错误如实返回。
8. 停止 PIE、重连、重复装配，无重复对象或丢失保存。
9. 关闭桥接后独立游戏包仍完整运行。

缺少小范围能力时补一个 Editor 辅助接口，不无期限寻找万能 MCP。

## 6. 为自主开发建设测试能力

### 可重复的灰盒装配

Lab、教程、挑战参数和事件绑定使用有版本的配置/脚本，按稳定 ID 更新对象，重复运行不复制 Actor；人工维护的美术区域与生成区域分开。脚本通过真实 UE 编辑器生成资产，不能以文本伪造 uasset。

Python 负责编辑器工作流，复杂 API 缺口由小型 C++ Editor 辅助补齐；正式技能和进度放运行时 C++。[UE5.7 Python 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/scripting-the-unreal-editor-using-python?application_version=5.7)。

### 输入驱动与状态快照（逐步建设）

- 状态：玩家位置/速度/运动状态、等级/冷却、挑战、计时、账本、epoch、机关占用。
- 操作：按时长或条件执行移动、跳跃、技能、交互，在引擎帧循环内推进。
- 等待：落地、门开启、挑战结束或超时，返回明确原因。
- 证据：输入序列、状态变化、截图、版本与配置，可直接复现失败。

本机已核对 `EnhancedInputSubsystemInterface.h` 中 `InjectInputForAction` 和 `InjectInputVectorForAction`。可据此做动作层测试，但它不能单独证明物理键位映射、操作系统焦点和全部 UI 正确；这些另走真实输入与窗口测试。

传送/改等级只建立测试前置条件。跳跃、碰撞、组合技能和最低配置路线必须走正式移动/技能代码。测试器属于开发设施，正式游戏不依赖它。

### 三层验证

| 层级 | 工作 |
|---|---|
| 规则 | 自动化验证边界时间、随机池、交换、去重和恢复状态机 |
| 场景 | Functional Tests 驱动角色/机关；截图检查镜头、落点、提示和 UI，动态问题需要连续状态证据 |
| 独立包 | UAT 打包，Gauntlet 组织启动、测试、退出和日志；正式包另验去掉开发依赖后的运行 |

Gauntlet 负责测试会话组织，不会自动理解本游戏，断言和通关驱动仍需我编写。[UE5.7 Gauntlet](https://dev.epicgames.com/documentation/en-us/unreal-engine/running-gauntlet-tests-in-unreal-engine?application_version=5.7)。性能问题用 [Unreal Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine?application_version=5.7) 实际采样。

## 7. 每次工作的固定闭环

读取任务/契约 → 实现与测试 → 构建 → 更新灰盒 → 运行观察 → 修复 → 保存证据与任务状态。

首个增量是出生、镜头、跳跃/位移、踩板开门、计时、掉落恢复和终点结算。其余能力接入同一奖励/恢复流程。保存接通后再验证退出重进，不提前声明完成。

将构建、装配、规则测试、场景测试、打包封装为可复现脚本，输出退出码、结构化结果和日志。通过状态轮询等待编辑器/构建完成，不用固定长等待猜测结果。

长期状态保存在项目任务表、决策记录、测试证据、问题清单和工具版本中，每次明确下一个可执行项；无需用户反复讲解项目。

默认一个主实现者控制工程、编辑器和构建。若后续明确使用多代理，可分配独立源码/只读审查，地图、蓝图和构建仍由一个负责人操作。本次未启动多代理。

## 8. 接入顺序与回滚

1. 确认目标工程。当前目录与同级其他工程关系未定，不直接修改其他项目。
2. 建立工程基线、Git/LFS、最小构建和独立包证据。
3. 引入精选 Skills：先说明目标、触发条件、组合、校验和回滚；使用项目级目录，固定来源版本。
4. 在测试工程验证首选 MCP，Codex 配置遵循官方格式，不照抄其他客户端 JSON。
5. 建立装配与输入/观察测试驱动，再扩展八项能力。
6. 沿既有任务依赖完成全部逻辑；正式美术制作与赛事资料另由团队承担。

回滚：工具使用项目级插件和独立环境；撤回 MCP 配置、禁用对应 Editor 插件、移除所选 Skill 即可退回终端/脚本流程。运行模块不依赖桥接，不修改引擎安装目录。实际安装后报告路径、启停入口和残留情况。

## 9. 已验证与未验证

- 已查官方文档、候选仓库和部分 Skill 源文，并核查本机引擎、工具及输入接口。
- 部分搜索摘要落后于真实文件，以打开的源文及本机版本为准。
- 插件目录搜索接口本次连接失败，目录层面可用性未确认；公开仓库调研已完成。
- 后续实测已安装四项 Skills、编译运行三套编辑器桥接、构建测试与正式工程、验证 PIE 并独立启动 Windows 包；T01 已通过 OS 键盘与基础相机验收，该条为工具基线阶段记录。当前完整逻辑进展见全逻辑实现与验收、Windows 交付说明。
- 工具方案作为原设计的新增补充，原始 v0.2 文档保留。
