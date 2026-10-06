# Tripothon 网页试玩部署（Windows + NVIDIA + FRP）

游戏运行在 Windows 主机上，网页和信令运行在该主机的 Docker Desktop Linux 容器内。公网 Linux VPS 提供 HTTPS、FRP 服务及 TURN 视频中继。迁移后开发电脑无需开机，目标机无需安装 UE 编辑器。

```text
玩家浏览器 ── HTTPS / WebSocket ── 公网 HTTPS 入口 ── frps ── frpc
                                                               │
                                                   Windows:8080 → Docker 网页/信令
                                                   Windows 游戏 → localhost:8888
玩家浏览器 ←──── WebRTC 视频、声音、输入 / 公网 TURN ─────────→ Windows 游戏
```

第一版限制为一个玩家连接一个实例。后来访问者不能得到第二份独立游戏；没有排队、账号或每人独立存档，访客共用当前游戏进度。网页断开不会重置游戏。不要把它当成已经实现了多人云游戏平台。

## 1. 在开发电脑打包

项目已启用 PixelStreaming2，必须重新打包，旧的普通 EXE 不会自动获得串流能力。在仓库根目录运行：

```powershell
./tools/scripts/package_game.ps1 -Configuration Shipping -Output "$PWD/artifacts/WebPlay/Game"
./tools/deploy/pixel-streaming/Build-Image.ps1
./tools/deploy/pixel-streaming/Export.ps1 -GameDirectory "$PWD/artifacts/WebPlay/Game/Windows" -Destination "D:/Tripothon-WebPlay" -IncludeImage
```

`Export.ps1` 要求新的目标目录，避免混入旧版本。`-IncludeImage` 导出信令镜像，目标机无需重新拉取依赖；不指定时，在目标机执行 `Build-Image.ps1`。若 Docker Hub 无法访问，可选用公开镜像源：

```powershell
./tools/deploy/pixel-streaming/Build-Image.ps1 -NodeImage public.ecr.aws/docker/library/node:22-bookworm
```

信令及官方播放器源码固定在 UE5.7 分支提交 `4fb38b9bbddecf2b7b3533fd740981893e3c532f`，使用其锁文件构建。网页当前使用官方播放器，没有额外设计网站首页。

官网离线下载一起迁移时，给 Export 增加 `-OfflineZip "C:/Tripothon_Build_20261006_CleanLevels/Tripothon-Windows.zip"`。同时使用 `-IncludeImage` 时，需要提前有 `nginx:1.28-alpine` 镜像（`docker pull nginx:1.28-alpine`）。镜像包将同时包含信令和下载服务。

## 2. Windows + RTX 5060 目标电脑

准备 NVIDIA 驱动、Docker Desktop（WSL2 / Linux containers）、VC++ 2015–2022 x64 运行库及你的 `frpc.exe`。保持 Windows 用户登录，先在本机验证；不要在最小化的远程桌面窗口里运行普通有窗口游戏。脚本以 `-RenderOffscreen` 离屏运行。

复制整个部署目录，例如 `D:/Tripothon-WebPlay`，在该目录打开 PowerShell：

```powershell
# 如果带了镜像：
docker load -i ./signalling-image.tar
# 否则： ./Build-Image.ps1
./Configure.ps1 -Mode Local
./Start.ps1
```

本机打开 `http://localhost:8080/`，点击播放器开始按钮，检查主菜单画面、声音、鼠标和键盘。Local 模式只用于本机验证；容器端口仅绑定 127.0.0.1，局域网其他电脑不能直接访问。

若本机端口已占用，配置时加 `-WebPort 18080 -StreamerPort 18888`，网页改访问 18080，并将 FRP 的 localPort 改为 18080。每次重新 Configure 时都要带上自定义端口，否则恢复默认。

默认 1920×1080、60 FPS 串流上限、H.264、自适应 2–15 Mbps 视频码率；实际帧率和码率取决于场景、网络及显卡。可用 `./Start.ps1 -Width 1280 -Height 720 -Fps 30` 降低串流负载。不改变项目的画质等级；游戏本身的渲染帧率还可在游戏设置中限制。网页默认 Hovering Mouse，适合菜单点击；自由视角需要锁定鼠标时可在播放器设置中切换 Control Scheme。

## 3. 公网 TURN 中继

只映射网页 8080 不等于视频可以跨公网连通。Public 模式配置 `iceTransportPolicy=relay`，强制经过 TURN，优先验证稳定连接。视频流量经过公网服务器，需有足够带宽和流量额度。

在 Windows 部署目录生成配置（替换示例地址）：

```powershell
./Stop.ps1
./Configure.ps1 -Mode Public -TurnHost turn.example.com -TurnPublicIP 203.0.113.10
```

该命令首次生成随机共享密钥，重复执行复用密钥。将 `public-turn/compose.yaml` 和生成的 `runtime/turnserver.conf` 放到公网 Linux VPS 的同一个目录，例如 `/opt/tripothon-turn/`，运行：

```bash
cd /opt/tripothon-turn
docker compose up -d
docker compose logs --tail 50
```

VPS 防火墙和云安全组放行：TCP/UDP 3478，以及 UDP 49160–49200。TURN 域名直接解析到 VPS，不能走普通 HTTP CDN 代理。这里使用 Docker host 网络，只适用于公网 Linux VPS。若 VPS 自身在 NAT 后，需要将生成的 `external-ip` 改为 `公网IP/本机内网IP` 并确保中继端口逐一映射。

`runtime/turn-secret.txt` 与 TURN 配置中的共享密钥必须一致；它不会发给浏览器，信令会签发有时效的 TURN 账号。配置文件已从 Git 忽略，迁移时私下复制。更换/丢失密钥后需要同步更新两端并重启。

如果你只有 FRP 端口映射权限，不能部署 TURN，则需要另一个可用的 TURN 服务。不要按“已有公网网页”判断完成部署。

## 4. FRP 和 HTTPS

将 `frpc.example.toml` 中的代理条目合并进你的现有 FRP 配置，填写服务器地址、认证信息和可用远程端口。推荐在 Windows 原生运行 frpc，它连接本机 `127.0.0.1:8080`。不映射 8888；该端口只用于游戏到信令。

示例链路：公网域名 HTTPS 443 → VPS 上 Caddy → `127.0.0.1:18080`（frps）→ Windows `127.0.0.1:8080`。下载链路则使用 frps 18081 → Windows 8081。`public-turn/Caddyfile.example` 适用于 Caddy 原生安装在 frps 所在 VPS 的情况，按 `/downloads/*` 分流。已有反向代理可以继续用，需支持 WebSocket 升级及较长连接超时。

开启 VPS TCP 80/443，并让游戏域名解析到 VPS。FRP TLS 只保护 frpc 到 frps，浏览器的 HTTPS 由公网反向代理提供。不要全局修改已有 frps 的 proxyBindAddr，除非确认不会影响其他代理；也可以只通过防火墙限制 18080 的公网访问。

然后在 Windows：

```powershell
./Start.ps1
# 在你现有的 FRP 管理方式中启动/重载 frpc
```

用手机热点下的另一台电脑访问 HTTPS 域名，进入游戏并确认音画和操作；同局域网访问不算公网验收。首次播放可能需要点击按钮解锁浏览器音频。

### 官网两个入口

部署目录存在 `Downloads/Tripothon-Windows.zip` 时，Start 自动启动 Nginx 下载容器。它支持 HTTP Range（断点续传）、Content-Length 和附件下载文件名。默认本机下载地址 `http://localhost:8081/downloads/Tripothon-Windows.zip`。端口冲突时 Configure 增加 `-DownloadPort 28081`，同步改 frpc 的 localPort。

公网部署后，交给官网任务填写实际配置：

```javascript
playUrl: "https://play.实际域名/"
downloadUrl: "https://play.实际域名/downloads/Tripothon-Windows.zip"
```

这里只给地址约定，不修改官网源文件。当前没有确定公网域名，官网必须保持待开放，不能把 localhost 链接发布给外部玩家。离线 ZIP 和串流 Game 是两个不同用途的包，更新时分别替换。

## 5. 停止、存档和迁移

```powershell
./Stop.ps1
```

停止会关闭此部署记录的游戏进程及 Compose 服务，不会删除存档。请在游戏完成保存后停止。

- `Game/`：打包程序及资源。
- `Downloads/`：可选离线 Demo ZIP，只读挂载给 Nginx。
- `Data/game/`：使用 UE `-UserDir` 指定的运行数据目录，存档位于其 Saved 子目录。
- `Data/signalling/`：信令日志。
- `runtime/`：网络配置、密钥和进程记录。
- `.env`：本机端口配置，迁移时一并保留。
- `signalling-image.tar`：可选离线镜像包，导入后可保留供迁移。

迁移：先停止，复制整个部署目录到另一台 Windows NVIDIA 电脑；安装运行环境、导入镜像、沿用配置并启动，再将 frpc 迁到新电脑。公网 TURN 地址不变时无需重配。启动脚本验证 PID、路径及启动时间，避免迁移后的旧 PID 记录影响其他进程。

移除部署：停止后删除该部署目录即可；Docker 镜像仍在 Docker Desktop 中，如不再使用可运行 `docker image rm tripothon-signalling:ue5.7-4fb38b9`。Nginx 镜像可能被其他项目共用，不自动清理。不会安装系统服务，也不会自动添加开机启动或改防火墙。

## 验收与故障定位

固定测试：Shipping / Win64，1080p60，从主菜单进入 Home，游玩 60 秒；再验证 School、lv4 的鼠标 UI、追逐和转场。目标机记录实际 FPS、RTT、丢包和主观操作延迟；建议以稳定 30 FPS 以上、无持续断连为首次可玩门槛，60 FPS 为目标。当前尚未测得 RTX 5060 的性能基线，也未证明公网延迟达标。

- 网页打不开：检查 `docker compose ps`、8080、frpc/frps、HTTPS 代理。
- 网页出现但无 streamer：检查 `docker compose logs --tail 100`、游戏进程、NVIDIA 驱动、游戏是否重新启用插件打包。
- 有 streamer 但黑屏/连接失败：检查 TURN 密钥、3478 与 relay 端口，播放器设置内观察 WebRTC 连接状态。
- 画面正常但手感差：检查 VPS 路由、带宽和玩家网络；先降低到 720p30 验证。
- 第二人无法进入：当前配置 `max_players=1`，属于预期。
- 此 Shipping 构建可能不生成详细游戏日志；需要诊断时重新打 Development，不能以不存在日志判断游戏未启动。

参考：[Epic UE5.7 信令源码](https://github.com/EpicGames/PixelStreamingInfrastructure/tree/UE5.7)、[FRP TCP/UDP](https://gofrp.org/en/docs/features/tcp-udp/)、[Coturn Docker](https://github.com/coturn/coturn/blob/master/docker/coturn/README.md)。
