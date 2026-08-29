# Spellblade 魔剑士 — UE5 动作 ARPG

[游戏演示视频](https://www.bilibili.com/video/BV1AH4C6wEb8/?share_source=copy_web&vd_source=92f3e22dd322afd6c14ab4ea29c7b8fb)

## 项目简介

基于 Unreal Engine 5.6 与 Gameplay Ability System 开发的第三人称动作ARPG，实现战斗系统，武器系统，敌人AI与波次生存玩法。因为Content文件过大所以没有推送到仓库。

## 核心玩法

- 轻重攻击连击系统（连击数影响伤害倍率，命中回怒气值）
- 特殊技能（跳跃攻击AOE、魔法飞弹：链式弹射、自动追踪、命中回怒气，均带冷却）
- 锁定（BoxTrace 扫描 + 叉乘判断目标左右 + 切换目标）
- 格挡（点积朝向判定）
- 怒气机制（怒气满了可以开狂暴模式，轻重击会生成火焰斩，魔法飞弹会从单发变成三连发）
- 波次生存模式（DataTable 驱动动态难度敌人配置）

## 技术亮点

- **三层 GAS 技能继承体系**：`WarriorGameplayAbility` → `HeroGameplayAbility` / `EnemyGameplayAbility`，基类提供激活策略（`OnTriggered` / `OnGiven`）
- **自定义 ExecutionCalculation**：`UGEExecCalc_DamageTaken` 实现伤害计算（武器基础伤害 × 连击倍率 × 攻击力 / 防御力），连击倍率通过 `SetByCaller` 动态传入（轻击最高 +15%，重击最高 +60%）
- **自定义 AbilitySystemComponent**：`Toggleable` / `MustBeHeld` 标签输入处理，武器技能批量授予/撤销
- **自定义 AbilityTask**：`WaitSpawnEnemies` 监听 GameplayEvent 异步加载并生成敌人；`ExecuteTaskOnTick` 供蓝图驱动每帧逻辑
- **5 个自定义行为树节点**：装饰器 `BTDecorator_DoesActorHasTag`（Tag 条件检查）、`BTDecorator_ShouldAbortAllLogic`（死亡/距离中止）；服务 `BTService_GetDistToTarget`、`BTService_OrientToTargetActor`；任务 `BTTask_RotateToFaceTarget`（Tick 插值旋转 + 点积角度检测）`UBTService_GetDistToTarget`更新获取目标距离等
- **GameplayTag 驱动全链路**：输入绑定、技能激活、事件通信、状态管理、动画切换、武器标识，统一由 GameplayTag 串联
- **魔法飞弹**：`BoxTraceMultiForObjects`扫描 + 重叠后切换下一个可攻击目标 + `Tick`每帧插值转向目标 + 命中后`HandleApplyInstigatorGameplayEffect`回玩家怒气
- **格挡判定 / 受击方向计算**：点积朝向验证 + 叉积方向检测，GAS 事件解耦视觉反馈
- **组件化架构**：战斗组件（`PawnCombatComponent` → `Hero/EnemyCombatComponent`）负责武器注册、装备与碰撞箱开关；UI 组件（`PawnUIComponent` → `Hero/EnemyUIComponent`）通过委托向蓝图广播属性变化与技能状态
- **接口分离**：`IPawnCombatInterface` / `IPawnUIInterface`，`GetOwningPawn<T>()` 模板 + `static_assert` 编译期类型安全
- **DataAsset 数据驱动**：角色启动数据、输入配置、技能赋予，武器数据、敌人波次均通过 DataAsset / DataTable 配置，新增内容不修改 C++
- **异步资源加载**：`TSoftObjectPtr` + `UAssetManager::RequestAsyncLoad`，角色数据与敌人波次非阻塞预加载

## 技术栈

`Unreal Engine 5.6` `C++` `Gameplay Ability System` `Enhanced Input` `Behavior Tree` `Motion Warping` `Niagara` `UMG` `NavigationSystem`

## 项目结构

```
Source/Warrior/
├── AbilitySystem/          # GAS 核心（技能/属性/伤害计算/技能任务）
│   ├── Abilities/          #   技能类（基础/英雄/敌人技能、目标锁定、拾取石头）
│   ├── AbilityTasks/       #   技能任务（每帧 Tick、监听事件生成敌人）
│   └── GEExecCalc/         #   伤害执行计算
├── Characters/             # 角色（基类 → 英雄/敌人）
├── Controllers/            # 玩家控制器 / AI 控制器
├── Components/             # 组件（战斗 / UI / 增强输入）
│   ├── Combat/             #   战斗组件
│   ├── UI/                 #   UI 组件
│   └── Input/              #   增强输入组件
├── AI/                     # 自定义行为树节点
│   ├── Tasks/              #   行为树任务
│   ├── Decorators/         #   行为树装饰器
│   └── Services/           #   行为树服务
├── AnimInstances/          # 动画实例
│   └── Hero/               #   英雄动画实例 + 链接动画层
├── DataAssets/             # 数据资产
│   ├── Input/              #   输入配置资产
│   └── StartUpData/        #   角色启动数据资产
├── GameModes/              # 基础 / 波次生存游戏模式
├── Interfaces/             # 战斗 / UI 接口
├── Items/                  # 武器 / 弹射物 / 魔法飞弹 / 拾取物
│   ├── PickUps/            #   拾取物
│   └── Weapons/            #   武器
├── Widgets/                # UI 控件基类
├── WarriorTypes/           # 枚举 / 结构体 / 自定义 Latent Action
└── SaveGame/               # 存档系统
```

## 操作说明

| 操作 | 按键 |
|---|---|
| 移动 | WASD |
| 镜头 | 鼠标 |
| 轻攻击 | 鼠标左键 |
| 重攻击 | 鼠标右键 |
| 装备/卸下武器 | Q |
| 拾取掉落物 | E |
| 开启狂暴模式 | R |
| 格挡 | 长按 Shift |
| 翻滚 | Space |
| 目标锁定 | 中键 |
| 切换锁定目标 | 鼠标左右移动（有死区） |
| 魔法飞弹 | 1 |
| 跳跃攻击AOE | 2 |

## 联系方式

[GitHub](https://github.com/r27etpl15) · 邮箱：1619433745@qq.com
