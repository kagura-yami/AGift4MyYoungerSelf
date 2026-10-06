# 本机验收记录 · 2026-10-06

## 已通过

- Win64 Shipping BuildCookRun，包含 PixelStreaming2，ExitCode=0。
- Docker 信令及官方播放器成功构建，固定 UE5.7 源码提交；Docker Hub 不可达时改用公开 ECR Node 基础镜像完成构建。
- 浏览器接收 1920×1080 H.264 画面，已点击菜单开始新旅程并进入 Home；Esc 能打开游戏内暂停菜单。
- 修正默认鼠标模式后，播放器默认 Hovering Mouse；自由视角可切换 Locked Mouse。
- 启用独立串流帧率后，本机重启后的 24 秒统计显示 59 FPS、0 丢包、0 丢帧。此数据是开发机回环连接，不代表 RTX 5060 或公网性能。
- Nginx 下载测试：HTTP 206、1024 字节，Content-Range 为 `bytes 0-1023/2209736615`，Content-Disposition 为 ZIP 附件文件名。
- 配置生成器已验证 Public 强制 relay、单玩家限制、密钥复用及切回 Local 清理 TURN 配置。
- PowerShell 脚本语法和 Compose 配置校验通过。
- 重复启动保护通过，Stop 已确认停止本部署游戏进程并移除两个测试容器及其网络。

## 交付位置

`artifacts/WebPlay/Portable/`：包含 Game、Downloads、离线 Docker 镜像及服务脚本。约 4.86 GiB。当前配置为 Local 模式，端口为网页 28080、游戏连接 28888、下载 28081，以避开开发机已有服务。

启动：先 `docker load -i signalling-image.tar`（迁移到新机时），然后 `./Start.ps1`。
停止：`./Stop.ps1`。当前测试服务已停止，没有驻留游戏进程。

未包含公网域名、FRP 认证信息及实际 TURN 密钥；按 README 在目标机配置 Public 模式。源码模板默认端口为 8080/8888/8081，当前交付目录中的 `.env` 是本机验证端口，部署时保持 FRP localPort 与之对应。

## 尚未验收

- 目标 RTX 5060 电脑的运行库、显卡性能与远程部署。
- 公网 FRP、HTTPS、TURN 端口及视频连通性。
- 外网追逐/跳跃手感、音频听感与全部关卡长时间游玩。
- 多玩家独立会话（本方案仅单实例共享进度，没有实现此功能）。

因此结论为：本机原型可运行，迁移包已生成；公网尚未上线。
