# 在线试玩与离线 Demo 入口

首页导航与首屏提供两条锚点入口，领取礼物区域提供实际外部链接。

部署服务验收通过后，填写 `dist/config.js`：

- `playUrl`：完整 HTTPS 在线播放器地址，可包含 `?HoveringMouse=true`。
- `downloadUrl`：完整 HTTPS ZIP 地址，路径由迁移方案约定为 `/downloads/Tripothon-Windows.zip`。
- `playLabel` / `releaseLabel`：开放后的状态文案。

空地址显示待开放；不接受开发机地址、明文 HTTP 或脚本协议。不要填写示例域名。修改后需要重新发布 Sites 版本，或将静态文件同步至实际官网服务器。

在线服务为单实例、共用进度，官网已明确标注。下载服务需返回 Content-Disposition: attachment，并支持 Range（206）以便大文件续传；跨域下载不能只依靠 HTML download 属性。

离线文件：`C:/Tripothon_Build_20261006_CleanLevels/Tripothon-Windows.zip`（2,209,736,615 字节）。不要把离线 ZIP 替换成专用串流运行包。

已完成桌面与 390px 手机版式检查，以及地址为空、无效地址和有效 HTTPS 配置的状态检查。公网域名未确定前，两个入口保持待开放。
