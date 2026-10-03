# LV4 电梯、巡逻与安全区

## 测试入口

UE **5.7.4**，打开 `/Game/Maps/L_Lv4MechanismWhitebox`，按编辑器播放按钮。

- 出生点向电梯走：将屏幕中央聚焦点对准入口按钮，亮起后按 **E** 或鼠标左键开门；进入轿厢后对准轿厢按钮，交互上下运行。靠近不再自动开门。
- **P** 打开暂停菜单；测试奖励面板也使用相同动作时钟暂停。
- 电梯南侧为三点往返巡逻路线，两个绿色房间可躲藏。正面进入视野会被追；藏进去立即免抓，仇恨每秒减 45。
- 出生区域同时有检查点和安全存档区。被抓后回到检查点，电梯和怪物一起恢复。
- 测试分身可用已有能力配置面板先授予分身能力，再按 C 创建。门口的未操控分身也会阻止关门；移入轿厢后可以随电梯运行。

## 可摆放蓝图

| 蓝图 | 原生类 | 用途 |
|---|---|---|
| `/Game/Blueprint/Lv4/BP_Lv4Elevator` | `ATripoElevator` | 两站移动平台、轿厢门、楼层入口门与井道阻挡 |
| `/Game/Blueprint/Lv4/BP_Lv4Patrol` | `ATripoChaser` | 巡逻、发现、追逐、搜索 |
| `/Game/Blueprint/Lv4/BP_Lv4HideZone` | `ATripoChaseHideZone` | 玩家可进入、怪物不可进入的藏身区 |

单机权威逻辑。没有添加联网复制支持。

### 电梯参数与组件

| 参数 | 默认 | 说明 |
|---|---:|---|
| `Stop0` / `Stop1` | `(0,0,0)` / `(0,0,600)` | 相对 Actor 的两个停靠位置，编辑器可拖动位置控件 |
| `InitialFloor` | 0 | 新游戏和没有电梯字段的旧存档使用的楼层 |
| `DoorSeconds` | 0.8 秒 | 单次开门或关门时长 |
| `AutoCloseDelay` | 1.5 秒 | 离开感应区后等待时间 |
| `TravelSpeed` | 200 | 厘米/秒，建议 Actor 保持单位缩放 |
| `LeftSlideDirection` / `RightSlideDirection` | `(0,-1,0)` / `(0,1,0)` | Actor 本地坐标下的左右平移方向，自动归一化 |
| `DoorTravel` | 105 | 平移距离 |
| `LeftClosedPosition` / `RightClosedPosition` | `(-155,±50,130)` | 轿厢门关闭时的位置 |
| `LandingDoorOffset` | `(-20,0,0)` | 入口门相对轿厢门的偏移 |
| `InteractionDistance` | 180 | 按钮交互距离，额外要求处于轿厢内且无遮挡 |

`Cabin` 是移动父组件；`Floor` 提供角色的移动平台基座。模型、照明、按钮提示可附加到 Cabin。门板沿平移方向移动，不能用门板 Actor 的旋转动画替代。模型尺寸变化时同步调整 `DoorSensor`、`CabinVolume` 和入口阻挡盒。

`DoorSensor` 同时检测主角、本体和所有分身的胶囊范围。按钮请求后如果门口有人，门会重新打开并等待清空；请求保留，期间再按 E 不重复启动。运行时两层入口均阻挡，到站才开放相应楼层的入口。

交互入口统一走角色 E。一次输入选择一个符合距离、视线要求的电梯；成功选择后不继续触发礼物盒、NPC 或开关。暂停、奖励暂停、恢复锁定期间拒绝交互。

### 巡逻和藏身

- `PatrolPoints`：按顺序放置 TargetPoint 引用。测试地图的点位放在胶囊中心高度（地面上约 90 厘米），第一点也是复活重置位置。
- `bStartPatrolling`：放置巡逻怪时设为 true；原生默认 false，保留旧触发器生成后 `StartChase` 的行为。
- `bPingPongPatrol=true`：0 → 1 → 2 → 1 → 0。false 则循环返回第一点。
- `PatrolWaitSeconds=1`：到点等待时间。没有路线时继续使用原有随机游荡。
- 保留 `SightRadius`、`SightHalfAngle`、`CloseDetectionRadius`、追逐速度比例、空中速度比例和出生保护时间。速度以角色正常步行速度为基准，不采用冲刺瞬时速度。
- `AggroReductionPerSecond=45`。0 沿用旧版“立即清空”语义；重叠区域取更强效果。
- 藏身区 `Volume` 标记 `UTripoHideNavArea`，`UTripoChaserNavFilter` 排除此区域；碰撞通道 `TripoChaser` 额外阻挡怪物胶囊，玩家 Pawn 可穿过。
- 布置后重建导航。需要运行时开关安全区时调用 `SetEnabled`，同步碰撞与导航状态。
- 怪物失去目标后先搜索；仇恨归零时选择最近巡逻点继续。失败恢复清空仇恨、运动和等待状态，返回第一巡逻点，重新开始出生保护。

## 正式 lv4 接入范围

读取了当前 `/Game/Maps/lv4` 的 1,022 个原始 Actor。

明确识别的西侧组：

- `SM_Bld_Elevator_01` / `StaticMeshActor_115`：原位置约 `(-1549.4,744.0,67.3)`。
- 下层门 `Cube`、`Cube2`；上层门 `Cube5`、`Cube6`。原门板 X 约 -1596.3，Y 约 506.4 / 623.6。
- 两个外观按钮 `SM_Prop_Elevator_Button_01`、`SM_Prop_Elevator_Button_2`，分别位于上下层，已接入聚焦呼叫：一楼上箭头、二楼下箭头。轿厢按钮负责上下运行。
- 原型接入 Actor：**`LV4_WestElevator`**，文件夹 `LV4_Mechanisms/WestElevator`。轿厢地板停靠世界高度为下层 Z=60、上层 Z=523.619，与实际楼板对齐。

原轿厢模型附加到移动 Cabin，原来的四块静止门板保留在 `OriginalPanels` 子文件夹，隐藏并关闭碰撞；运行门板复用其网格、尺寸与材质。未删除原模型，没有修改外侧房间、主通道和其他电梯对象。

另一个 `SM_Bld_Elevator_2` 位于约 `(2114.2,-2472.0,514.8)`，不与该竖井上下对齐，未擅自连接。原关卡没有发现可直接对应本批巡逻怪物和路线的导航布局；正式怪物路线通过上述蓝图交由地编摆放，完整演示在白盒地图中。

已识别的 NPC 外观与提示物：

- `SK_Character_BusinessMan_Suit`：约 `(1315.9,1122.9,520)`。
- `SK_Character_BusinessMan_Suit2`：约 `(-290.8,149.1,520)`。
- `SK_Character_BusinessMan_Suit3`：约 `(1031.3,-1525.4,563.7)`。
- 场景还有 4 个加速泡泡、2 个减速泡泡和 3 个感叹号提示蓝图。这些对象无法仅凭名字确定录音中的敌我身份或任务关系，本批保持原样。

## 存档与恢复

- `FTripoCheckpoint`、`UTripoSaveGame` 新增可选 `ElevatorFloors`，键为 Identity 稳定 GUID，值只能是 0 或 1。存档版本仍为 1。
- 任何电梯处于关门准备运行或升降状态，都禁止创建检查点及安全存档。
- 检查点恢复和安全读档先恢复电梯，再验证玩家落点；只恢复停靠层，门关闭、运行请求清空。
- 旧存档没有该字段或没有对应 GUID 时使用 `InitialFloor`。旧出生点不安全时仍沿用已有出生点回退逻辑。
- 本轮磁盘存档测试临时隔离了 Story_A/B，完成后已恢复原文件并核对 SHA-256。

## 验证记录

- UE 5.7.4 Development Editor 编译成功。
- 自动化 22 项通过，包括新增的入口保护/状态恢复、序列化兼容和导航隔离测试。
- `validate_lv4_pie.py`：31 项跨帧检查通过，覆盖手动开门、延迟关门、防重入、分身防夹、主角/分身上下承载、菜单及奖励暂停、失败恢复、巡逻感知、藏身与重新发现。
- `validate_lv4_extra_pie.py`：9 项通过，覆盖到点等待、路线折返、墙体遮挡、真实抓捕触发复活、空井入口阻挡和远距交互拒绝。
- 原 `/Game/Maps/L_ChaseWhitebox` 的 21 项跨帧回归检查通过。
- 实际磁盘保存并 ContinueGame 重新加载，记录的上层正确恢复；模拟缺失电梯字段的存档正确使用初始层。
- 使用真实键鼠 E 启动电梯，观察角色随平台运行。正式 lv4 检查了下层入口、上层到站及出口碰撞通行。

验证数据保存在 `tools/evaluation/lv4-*.json`、构建日志和截图中。磁盘测试的旧 Python 反射步骤因字段不可写停止；后续通过独立测试槽的缺失字段序列化夹具完成了旧存档实际加载验证，记录在 `lv4-old-save.json`。

关卡修改前的备份位于 `tools/evaluation/lv4-backup/`。本轮没有自动提交或推送 Git。

## 复现脚本

- `tools/scripts/create_lv4_mechanism_whitebox.py`：仅首次生成白盒，已有地图时拒绝覆盖。
- `tools/scripts/integrate_lv4_elevator.py`：西侧电梯首次接入，先备份；已存在 `LV4_WestElevator` 时拒绝再次覆盖。
- `tools/scripts/validate_lv4_pie.py`、`validate_lv4_extra_pie.py`：在白盒 PIE 中运行，仅改变运行时世界。

密码谜题、线索卡、正式 NPC 台词、能力奖励以及路线封锁不属于本批实现。

## 2026-10-04 聚焦交互与碰撞修订

- 准星选中一个目标后高亮，并显示交互提示。电梯按钮、礼物盒和机关开关接入同一 `UTripoInteractionTarget` 组件。其他蓝图可添加该组件，配置 `HighlightMesh`、`Prompt`、`Reach`，绑定 `OnInteract`。
- 聚焦检查镜头射线、角色距离和角色到目标的遮挡。E 或鼠标左键操作选中的目标；菜单、奖励或恢复期间拒绝交互。原剧情区域和交换菜单保留原有 E 入口。
- 一楼只接上箭头，二楼只接下箭头。`LandingControls[0/1]` 是楼层呼叫，`CabinControl` 是上下运行。远端呼叫会先关门、到对应楼层再开门。
- 默认 `bManualDoors=true`。站在关闭的门前不会开门；已经开门时仍然检测主角和分身防夹。
- `UpperDoorOffset` 保留为可选调整参数，当前 LV4 为零；`LandingDoorOffset=(5,0,0)` 分离层门和轿厢门轨道。轿厢门保持可见，在运行期间遮挡井道。

## 2026-10-04 电梯修整

- 从原模型分离固定门框与移动轿厢，两层各有独立门框，不随升降移动；原始源模型保留。
- `LandingButtonActors` 绑定实际墙面按钮，`LandingButtonOffsets` 定义上/下箭头位置，避免移动电梯 Actor 后交互热点错位。
- 轿厢按钮避开扶手，取消悬空 E/SHIFT 提示，统一使用聚焦提示。
- 运行速度 160 厘米/秒，加减速度 140 厘米/秒²；门完全打开后才开始 3 秒自动关闭计时。门口防夹区收窄到门槛，轿厢内正常站立不会持续挡门。
- 重建脚本：`tools/scripts/rework_lv4_elevator.py`；修改前备份位于 `tools/evaluation/elevator-rework/`。
- 正式 LV4 往返 PIE 验证通过：两层聚焦呼梯、轿厢交互、升降承载、运行中门关闭、固定门框、暂停恢复、到站高度和二楼离开后关门。报告：`tools/evaluation/elevator-rework/journey.json`。
- 二楼 `LV4_UpperDoorDashEntry` 朝门内，半径 85cm，与 `LV4_UpperDoorDashExit` 成对。入口要求朝向点积至少 0.7、轿厢停在二楼；仅入口可触发。目标胶囊空间被占用时拒绝传送，不关闭全局碰撞。普通前闪仍采用带碰撞的移动。
- LV4 的 119 个无物理资产墙面生成独立刚性碰撞副本，位于 `LV4_Mechanisms/WallCollision`；严格保留原模型门洞。墙面静态资产及轿厢使用独立碰撞资源 `/Game/Blueprint/Lv4/Collision`，未改原始美术资产。
- 镜头关闭位置滞后，探测球半径 20cm；抬高视线并略偏肩，避免角色头部遮挡聚焦点。
- 本批关卡修改前备份：`tools/evaluation/focus-backup`。修复脚本：`refine_lv4_interactions.py`、`solidify_lv4_walls.py`；测试：`validate_focus_pie.py`。

交互视觉：高亮采用浅蓝色半透明覆盖（Opacity 0.24）；操作提示在聚焦点下方居中，E 键使用独立键帽。

### 轿厢内侧封闭与材质修正

- `seal_lv4_elevator.py` 在上述重建脚本之后执行：轿厢门局部 X=-90，层门偏移 X=-38.696，将固定层门移至轿厢门外侧，防止楼层结构在升降时穿入视野。
- 增加随 Cabin 移动的门楣、侧封板与门槛；门板宽度修正为 117.2 厘米，消除中间透光间隙。
- 门板替换默认白色材质，使用独立粗糙金属材质；不修改源模型或公共材质。
- 半层仰视截图：`tools/evaluation/elevator-rework/sealed-final00000.png`。固定层门不再横向穿过轿厢门。

### 门缝与门顶复查

- 门板宽度改为 116.364 厘米，恢复 8 毫米中缝；深色密封条跟随左门运动，阻挡中缝后的景物。
- 轿厢门楣加深至 80 厘米，两层增加固定门楣，封闭近距离仰视时暴露的门顶通道。
- 视觉证据：`seam-mid00000.png`（半层轿厢内）、`seam-out00000.png`（入口仰视），位于 `tools/evaluation/elevator-rework/`。

- 按钮高亮修正：`fit_lv4_button_highlight.py` 提取原模型上下按钮正面的多边形，覆盖层离表面约 0.07 厘米；一楼只亮上按钮，二楼只亮下按钮。底板保持原材质。已完成两层聚焦、呼梯及画面验证（`arrow-final00000.png`、`arrow-down-final00000.png`）。
