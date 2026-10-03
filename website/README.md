# 游戏宣传网站

静态官网，沿用游戏的回忆记录册配色与真实公司场景，包含游戏介绍、八项能力手册、入门攻略、下载安装及 tripothon参赛队介绍。

- 页面：`dist/index.html`
- 配色、首屏与响应式基础：`dist/styles.css`
- 栏目样式：`dist/sections.css`
- 技能数据与交互：`dist/app.js`
- 下载地址：`dist/config.js` 的 `downloadUrl`，仅接受 HTTPS。空值展示“即将开放”；填写后自动显示下载按钮，可同步修改 `releaseLabel`。

## 本地预览

在本目录运行 `python -m http.server 18793 --bind 127.0.0.1 --directory dist`，打开 http://127.0.0.1:18793 。Ctrl+C 停止。本网站没有构建依赖，也可离线打开 HTML。

## 内容依据

场景图片来自 `docs/ui-style/scene.png`，技能图标来自 `docs/ui-style/skill-icons-transparent.png`。玩法文案参考当前技能与 UI 设计基线、分身功能说明及剧情脚本。未使用固定技能数值，避免与关卡配置不一致；未公开故事结尾。关于团队是待团队审阅的创作理念文案，没有虚构成员、奖项或联系方式。

## 发布

Sites 项目标识保存在 `.openai/hosting.json`，静态目录为 `dist`。当前发布为所有者私有预览；对外开放需另行更改网站分享范围。后续编辑后需重新发布。

## 插画版本（2026-09-30）

使用内置 imagegen 生成三张统一色调的宣传概念插画：`hero-v2.png`（礼盒回忆世界）、`school-v2.png`（漂浮教室）、`parcel-v2.png`（寄给过去的包裹）。页面明确标注概念插画，不作为实机截图。原有能力图标继续复用。生成素材位于 `dist/assets`，无需外部图片服务。
