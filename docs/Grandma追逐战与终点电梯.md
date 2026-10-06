# Project Context Analysis

## Analysis Purpose
为 LV4 追逐敌人替换 Grandma 外观，接入待机/追逐动画，修复终点电梯通行并清空仇恨。

## Codebase Structure
原有逻辑位于 `game/Source/Tripothon/World/TripoChaser.*`、`TripoChaseHideZone.*`、`TripoChaseTrigger.*`；关卡为 `/Game/Maps/lv4`。触发器新增可配置的 `ExitZones` 引用数组，进入安全区调用原有 `EndChase(false)`，销毁它拥有的追逐敌人，不影响独立巡逻角色。

## Dependencies
Grandma 与 Son 的全部骨骼名称、局部参考平移和旋转一致。登记双向骨架兼容，复用 `TripoLocomotionAnimInstance` 及其 Idle/Move/Jump/Fall 动画。独立材质 `MI_Grandma_Chase` 复用项目现有 Office 调色板贴图，不改共享材质。

## Connections
办公室触发器生成 `BP_OfficeDoorChaser`；蓝图默认 Mesh 已替换 Grandma。关卡内 `LV4_CorridorPatrol` 同步替换。模型缩放 0.85、Z 偏移 -88、Yaw -90，隐藏白盒外观。

## Architecture Patterns
沿用单机 AI 导航、速度驱动动画和现有触发器 Tick。办公室触发器关联两个安全区：终点电梯与新增安全屋。进入任一区域后本轮办公室追逐结束，离开不会立即重生；检查点恢复仍按原有规则重新布置遭遇。其他追逐敌人继续使用隐藏区的清仇恨逻辑。

## Codebase Conventions
作者脚本位于 `tools/scripts/setup_grandma_chase.py`，运行验证脚本位于 `tools/scripts/validate_grandma_chase.py`。运行前备份到 `tools/evaluation/grandma-chase/<时间>/`。

## Dependency Graph
`OfficeDoor trigger → BP_OfficeDoorChaser → Grandma mesh + MI_Grandma_Chase + TripoLocomotionAnimInstance`

`Player position → LV4_ChaseEndElevator_SafeZone → TripoChaser aggro / search memory`

## Component Relationships
终点电梯为 `SM_Bld_Elevator_2`，不是西侧升降电梯。使用独立网格 `/Game/Blueprint/Lv4/Collision/SM_ChaseEndElevator_Open`，移除简单碰撞并使用双面三角面碰撞，保留轿厢地板、墙壁和入口真实形状。

安全区中心 `(2280,-2300,715)`，半尺寸 `(150,135,175)`；忽略 Pawn，仅阻挡 TripoChaser；仇恨消除速度 0 表示即时归零。隐藏调试标签。

## Key Insights
修改前 Grandma 材质实例未绑定贴图，办公室追逐蓝图未配置动画类。骨架完全相同，可直接兼容，无需重定向或另造动作。

初版 17 项 PIE 检查保留在历史报告；其中退出追踪但继续游荡的预期，已由当前办公室追逐的销毁行为取代。追加检查验证：主角独立材质、巡逻速度 226.8 cm/s（原 189，提升 20%）、安全屋与电梯分别销毁办公室敌人、巡逻敌人仍存在、离开后不立即重触发。Editor C++ 构建通过。

主角使用 `M_Son_Colored` 和根据原始 UV 色块制作的 `T_Son_Palette`，自然肤色、暖色衣服、深蓝裤子。`TripoCharacter` 移除办公室材质覆盖，直接使用骨骼网格上的材质；已显式启用 SkeletalMesh 材质用途。材质与关卡配置脚本为 `setup_player_chase_refinement.py`，调色板源码为 `create_son_palette.py`。

报告：`tools/evaluation/grandma-chase-validation.json`；视觉检查截图：`tools/evaluation/grandma-character.png`。测试结束已退出 PIE，保存关卡与资源；未重新打包发行版。

手工复验：运行 lv4 → 经过办公室门口触发追逐 → 进入安全屋或终点电梯 → 办公室老奶奶消失，走廊巡逻角色保留。当前报告为 `tools/evaluation/player-chase-refinement.json`，主角截图为 `tools/evaluation/son-colored-final.png`。未重新打包发行版。
