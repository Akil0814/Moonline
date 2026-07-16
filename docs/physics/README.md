# 物理与 Gameplay 碰撞框架

## 当前状态

本目录记录 Moonline 的物理与 Gameplay 碰撞框架边界。当前阶段只提供可编译的数据契约和空系统入口，尚未实现：

- 运动积分、重力和阻尼；
- 粗检测、矩形相交和空间索引；
- 阻挡、穿透修正和接地判断；
- PushBox 推挤；
- HitBox/HurtBox 配对、命中去重和伤害结算。

因此，当前 `PhysicsSystem::step` 和 `CollisionSystem::dispatch_events` 调用不会改变对象或产生碰撞事件。

## 分层边界

### `engine/physics`

物理核心只认识 Collider、矩形、过滤位、响应类型和接触数据，不认识 Actor、玩家、敌人、阵营、攻击或伤害。

- `ColliderId` 是 Collider 的稳定标识；零值表示无效 ID。
- `CollisionFilter::category` 表示 Collider 所属类别，`mask` 表示候选对象类别，`group` 为后续成组过滤预留。
- `CollisionResponse` 描述 Ignore、Overlap 或 Block 意图。
- `CollisionOverlap` 和 `CollisionContact` 只记录双方 Collider 及几何结果。
- `Collider::tag` 仅用于调试，不参与碰撞规则。
- `Collider::is_trigger` 暂时保留用于源代码兼容；新代码应使用 `CollisionResponse::Overlap`。

具体 category 位由使用物理核心的上层模块定义。物理核心不得加入 Player、Enemy、HitBox 等固定类别。

### `engine/gameplay_support/collision`

Gameplay 便利层把通用 Collider 绑定为 Actor 相关语义：

- `ColliderBinding`：Collider 所属 Actor、Team 和 Role；
- `ActorCollisionRig`：一个 Actor 的 Body、PushBox、HurtBox 和 Sensor 集合；
- `HitBoxBinding`：HitBox 的 owner、instigator、攻击实例和攻击定义；
- `TeamRelationResolver`：由项目决定两个 Team 是 Friendly、Neutral 还是 Hostile；
- `GameplayCollisionListener`：预留 Body、PushBox 和命中事件入口；
- `GameplayCollisionService`：预留绑定的注册与清理契约，不提供默认实现。

`teams::Player`、`teams::Enemy` 和 `teams::Neutral` 是常用预设，不限制项目创建更多 `TeamId`。敌对关系不能通过 Team 数值直接推断，必须由 `TeamRelationResolver` 决定。

## 来源与目标

物理接触本身没有攻击方向，只包含一对 Collider。Gameplay 层按 Role 将事件规范化：

```text
Body     <-> World    -> BodyContactEvent
PushBox  <-> PushBox  -> PushBoxOverlapEvent
HitBox   ->  HurtBox  -> HitOverlapEvent
```

普通 Collider 的来源由 `ColliderBinding::owner` 表示。攻击的责任来源由 `HitBoxBinding::instigator` 表示；这允许飞行道具拥有自己的 Collider，同时把命中归属给发射它的 Actor。`attack_instance` 用于未来的一次攻击内命中去重，`attack_definition` 用于查找招式数据。

过滤和语义判断分两阶段进行：

1. 物理核心根据 category/mask 产生可能接触的 Collider 对。
2. Gameplay 层查询 binding 和 Team 关系，将 Collider 对路由到 Body、PushBox 或战斗处理器。

## 后续实现顺序

1. 为矩形 Collider 实现世界坐标计算、category/mask 过滤和相交结果。
2. 实现 PhysicsBody 积分以及 Body 与世界的阻挡、接地和墙体处理。
3. 实现 Gameplay binding 存储和 PushBox 专用解算器。
4. 实现 HitBox/HurtBox 路由、Team 过滤和攻击实例命中去重。
5. 在项目 `gameplay` 层实现伤害、硬直、击退和具体角色规则。

每个阶段都应先补齐单元测试，再接入下一个阶段；不得把 Gameplay 语义下沉到 `engine/physics`。
