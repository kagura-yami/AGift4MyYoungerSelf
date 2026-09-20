# 本地开发工具

- `scripts/probe_mcp.py`：隔离环境内的 MCP 协议实测客户端；保存完整返回，图片另存 PNG。进程成功不代表业务成功，检查 `isError` 及嵌套结果。
- `scripts/setup_probe.py`：生成一次性 Probe 工程。会重写 Probe 源码；只用于重新建立实验环境。
- `scripts/select_probe_bridge.py`：显式选择一个实验桥接，其余禁用。
- `scripts/run_automation.ps1`：运行 UE Automation，要求有报告、有成功用例且无失败，超时失败。
- `scripts/setup_tools.ps1`：建立正式 MCP 客户端环境；按 requirements.lock.txt 安装。
- `scripts/unreal.py`：主选 HTTP MCP 的脚本入口，解析业务失败并返回非零退出码。
- `scripts/build_game.ps1` / `open_editor.ps1` / `package_game.ps1`：正式游戏构建、打开、打包。
- `scripts/api_offline.py`：实验性 API MCP 离线入口；需要 evaluation 中的源码索引与依赖，不是正式游戏依赖。
- `evaluation.lock.json`：所有测试候选的提交 SHA；`patches/` 保留兼容差异。重新生成 Probe 后，如选择 remi，需要按记录应用 Fab 修正。
- `evaluation/`：忽略版本控制的克隆、虚拟环境、构建、截图和原始日志。

引擎基线：`C:/Apps/UE/UE_5.7`（5.7.4）。本机 C++ 构建使用 `-MaxParallelActions=2 -NoUBA`，避免并行 PCH 导致虚拟内存不足。依赖和实测结论见 `docs/工具实测报告.md`。

- `scripts/collect_logic_evidence.py`：检查通过状态后归档全逻辑报告和源码/资产/EXE 哈希。
- `package_game.ps1 -Configuration Shipping -Output artifacts/LogicShipping`：生成 Shipping 包；默认 Development。两种包均明确 Cook 教程和实验场。
