# 候选工具兼容差异

仅用于保留本次实测经过，不是正式游戏依赖。主选 Avatar 插件未修改源码。

- `remi-fab.patch`：本机没有 Fab，实验插件用目录存在性检查代替无条件 Editor 依赖。
- `ronildo-sdk.patch`：FastMCP description 改为 instructions，两项图像工具禁用结构化返回 schema 自动推导；同时 Python SDK 固定 1.x。
- `chir-module-detection.patch`：依据真实 Build.cs 判断可选模块，避免仅因 Binaries 下同名目录误判；修正后仍有 C2026，未通过原生编译。

源版本见 `tools/evaluation.lock.json`；这些差异未向上游发布。
