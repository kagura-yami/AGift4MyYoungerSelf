# GASP 参考与视角主导控制（当前规则）

2026-09-15。用户再次试玩指出：正方向应由视角转动决定。此规则取代上一版“身体带镜头”和连续移动冻结基准的实现。

## 参考范围

已阅读 [Epic GASP 5.7 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/game-animation-sample-project-in-unreal-engine?application_version=5.7) 与 [5.7 更新说明](https://www.unrealengine.com/tech-blog/explore-the-updates-to-the-game-animation-sample-project-in-ue-5-7)。官方控制说明将鼠标旋转镜头、自由旋转/横移模式切换、聚焦分别定义。

项目目录内未发现安装的 GASP 工程。本次参考的是官方说明中的控制职责划分；没有声称检查过 GASP 蓝图源码，也没有迁移 Motion Matching、Mover 或动画资产。当前使用现有 CharacterMovement 完成视角主导的横移控制。

## 当前控制关系

鼠标 → Controller.ControlRotation → 镜头；视角朝向 → 身体跟随与移动输入。

- 鼠标水平旋转可持续超过 35°，以 360° 环绕；不再卡在角色周围的局部角度。
- SpringArm 使用 Pawn ControlRotation，角度不继承身体旋转。身体旋转不会反写视角。
- 身体平滑跟随视角，默认最大角度落后 35°、追随速度 360°/秒。35° 是身体落后量而非镜头转动上限，可调为 0 使身体同向。
- W 沿当前视线的水平前方移动；A/D 横移；S 后退。按住 W 时转视角可以直接改变行进方向，不需先松键。
- 空中鼠标同样控制视角，新移动输入使用当前视线；既有速度仍由 CharacterMovement/AirControl 积分，没有因镜头转动直接旋转已有速度。
- 鼠标上下控制俯仰，默认范围为低头 −75° 至抬头 +60°；初始与重置俯角为 −35°。左右旋转保留当前俯仰角。
- 身体与 WASD 移动只使用视角 Yaw，不使用 Pitch/Roll；抬头低头不会改变平面移动速度、方向或角色高度。前向锥体保留，标示身体实际 +X。
- 旧相机区域仅保留为身份测试载体，不控制镜头。

## 验证

2026-09-16：新增 `tripo_pitch_verify.run(report_path)`，12 项 PIE 输入动作检查通过。实际 CameraManager 上下角度、左右旋转保留俯仰均正确；−75°、0°、+60° 下前进速度均为 450 cm/s，竖直速度为 0，地面高度一致。12 项原有 Automation 全部通过。报告：`tools/evaluation/reports/pitch-runtime.json`、`pitch-automation/index.json`。

15 项 PIE 检查通过：视角超出旧上限、实际 CameraManager 朝向、原地身体跟随、身体外部旋转不影响镜头、跨 180°、按住 W 转视角、横移、后退、空中视角与输入。

测试入口：`tripo_view_verify.run(report_path)`。旧 facing/camera_airborne 入口转接本测试；旧归档保留为历史，不代表当前手感验收。

2026-09-15 用户试玩反馈：“我检查是没问题应该，继续开发吧”。本版控制据此通过当前手感验收。自动化的最后一次实际鼠标检查被 Esc 中止，不计为通过；用户反馈与数字测试分别归档。
