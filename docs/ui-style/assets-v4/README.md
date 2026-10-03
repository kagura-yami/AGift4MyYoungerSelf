# 透明菜单素材 v4

重新生成为真实 RGBA 透明贴图，纸本、书页、信封的自然轮廓外无实心矩形背景。图片不包含标题、正文、图标或按钮；内容由 Slate 实时绘制。

| 文件 | UE 纹理 | 用途 |
|---|---|---|
| pause-alpha.png | T_UI_PauseAlpha | 暂停菜单 |
| book-alpha.png | T_UI_BookAlpha | 能力手册 |
| paper-alpha.png | T_UI_PaperAlpha | 设置、教程、更多选项、开始菜单 |
| upgrade-alpha.png | T_UI_UpgradeAlpha | 升级、能力配置、能力交换 |
| item-alpha.png | T_UI_ItemAlpha | 物品及纪念物 |
| dialogue-alpha.png | T_UI_DialogueAlpha | NPC 对话 |

资源目录：/Game/UI/Prototype。通过 tools/scripts/prepare_menu_alpha.py 在 UE Python 中导入，保留 Alpha 并使用 UI 纹理压缩。

inspection.json 记录实际尺寸、透明像素比例与四角透明度。六张均有大于 10% 的完全透明像素，四角 Alpha 不超过 2/255。旧素材保留为历史参考，运行时菜单已切换至 Alpha 版本。

面板容器使用透明 SOverlay 与完整贴图；不再拉伸旧素材的一小条纸纹作为矩形 SBorder。按钮使用圆角样式。

## 字体与排版

- 页面标题和能力名称采用思源宋体 SemiBold，正文、按钮及辅助信息采用思源黑体 Regular。
- 字体原文件和 OFL 许可证在 `game/Content/UI/Fonts`，通过 UFS 随包分发，无需玩家安装字体。
- `TripoMenuStyle.h` 集中定义标题字距、1.22 倍正文行高、墨色/辅助色、细分隔线及键帽。
- 暂停页突出继续游戏，其余操作轻底显示；操作教程按键与说明对齐，能力手册入口固定在页脚。
- 能力手册区分目录、名称、等级/按键、说明、当前效果；对话页区分发言人和正文，选择横向排列。
- 短按钮禁用自动换行；长说明允许换行。超出可视区的内容可滚动，保留纸张装饰安全边距。

验证：UE 5.7.4 Editor 编译、8 页 PIE 截图检查；打包目录配置已加入，但本轮未执行完整发布打包。

## 菜单输入修复

- 复现：在 lv4 的 PIE 暂停页/能力配置页点击按钮无响应；分身轮盘关闭时仍存在。
- 根因：轮盘叶子虽然 HitTestInvisible，位于菜单上层的 SScaleBox 仍参与鼠标命中，遮挡中心区域按钮及滚轮。
- 修复：整个轮盘缩放容器设为 HitTestInvisible，保留布局尺寸和轮盘绘制；轮盘仍由玩家控制器读取 C/X 与鼠标输入。
- 三处菜单滚动容器开启 AnimateWheelScrolling，滚轮倍率 0.7，并始终消费菜单区域的滚轮，避免到底后传给场景。
- 按钮采用思源宋体，正文保留思源黑体。字体已附带，无需购买或额外安装。

本轮验证：Editor 编译成功；通过 Windows 鼠标输入实际点击暂停→更多选项→能力配置，滚轮滚动到底，再点击返回菜单成功。验证未执行新游戏、保存或能力修改。

## 设置页图文入口

设置页改为两个可点击入口：WASD 立体键帽对应操作指南，三枚现有能力徽章对应能力手册。说明缩短，整行可点击，装饰子树鼠标穿透。统一字体升级为思源宋体 Bold + 思源黑体 Medium，保留原 OFL 许可，旧字重仅作历史资源保留。
