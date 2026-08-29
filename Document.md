# Spellblade 项目文档

## 1. 项目概览

| 属性 | 值 |
|---|---|
| 项目名称 | Spellblade |
| 引擎版本 | Unreal Engine 5.6 |
| 游戏类型 | 动作 ARPG |
| 核心系统 | GAS（Gameplay Ability System）、Enhanced Input、Behavior Tree AI |

### 1.1 模块依赖

```
Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayTags,
GameplayTasks, GameplayAbilities, AnimGraphRuntime, MotionWarping,
Niagara, NavigationSystem, MoviePlayer
```

### 1.2 启用的插件

- **GameplayAbilities** — GAS 能力系统
- **MotionWarping** — 动画运动扭曲
- **ModelingToolsEditorMode** — 编辑器建模工具
- **VisualStudioTools** — VS 集成

---

## 2. 架构总览

```
Source/Warrior/
├── Warrior.h / .cpp                    # 模块入口
├── Warrior.Build.cs                    # 构建配置
├── WarriorGameplayTags.h / .cpp        # 全局 GameplayTag 声明
├── WarriorGameInstance.h / .cpp        # GameInstance
├── WarriorBlueprintFunctionLibrary     # 蓝图工具函数库
├── Characters/                         # 角色层
├── Controllers/                        # 控制器层
├── AbilitySystem/                      # GAS 能力系统
│   ├── Abilities/                      #   能力类
│   ├── AbilityTasks/                   #   能力任务
│   └── GEExecCalc/                     #   伤害执行计算
├── Components/
│   ├── Combat/                         #   战斗组件
│   ├── UI/                             #   UI 组件
│   └── Input/                          #   增强输入组件
├── AI/                                 # 行为树节点
│   ├── Tasks/
│   ├── Decorators/
│   └── Services/
├── AnimInstances/                      # 动画实例
│   └── Hero/
├── DataAssets/                         # 数据资产
│   ├── Input/
│   └── StartUpData/
├── GameModes/                          # 游戏模式
├── Interfaces/                         # 接口
├── Items/
│   ├── PickUps/                        #   拾取物
│   └── Weapons/                        #   武器
├── Widgets/                            # UI 控件
├── WarriorTypes/                       # 枚举/结构体
└── SaveGame/                           # 存档
```

---

## 3. 核心框架

### 3.1 `UWarriorGameInstance` → `UGameInstance`

管理关卡与 GameplayTag 的映射表，支持按 Tag 查找关卡引用。绑定全局地图加载事件以显示/关闭加载画面（最低显示 2 秒）。

| 成员 | 说明 |
|---|---|
| `GameLevelSets` | 关卡集合（Tag → Level 软引用映射） |
| `GetGameLevelByTag(FGameplayTag)` | 通过 Tag 查询关卡 |

### 3.2 `AWarriorBaseGameMode` → `AGameModeBase`

基础游戏模式，存储当前游戏难度。

| 成员 | 说明 |
|---|---|
| `CurrentGameDifficulty` | 游戏难度（Easy / Normal / Hard / VeryHard） |

### 3.3 `AWarriorSurvivalGameMode` → `AWarriorBaseGameMode`

波次生存模式，通过 `UDataTable` 驱动敌人波次生成。内置**状态机**：

```
WaitSpawnNewWave → SpawningNewWave → InProgress → WaveCompleted →
  (AllWavesDone ✓ / 下一波 WaitSpawnNewWave / PlayerDied ✗)
```

| 关键逻辑 | 说明 |
|---|---|
| 异步预加载 | `PreLoadNextWaveEnemies()` 通过 `UAssetManager` 异步加载下一波敌人 |
| 随机生成 | 在目标点周围导航可达范围内随机生成敌人 |
| 难度缩放 | 英雄等级随难度降低而提升，敌人等级随难度提升而提升 |
| 外部注册 | `RegisterSpawnedEnemies()` 支持召唤出的敌人纳入波次追踪 |

### 3.4 GameplayTag 体系 (`WarriorGameplayTags.h/cpp`)

所有 Tag 集中在 `WarriorGameplayTags` 命名空间，遵循以下分类：

| 前缀 | 用途 | 示例 |
|---|---|---|
| `InputTag_*` | 输入映射 | Move, Look, LightAttack, HeavyAttack, Roll, SwitchTarget |
| `Player_Ability_*` | 玩家能力 | PickUp_Stones |
| `Player_Cooldown_*` | 玩家冷却 | |
| `Player_Event_*` | 玩家事件 | HitPause, SuccessfulBlock, SwitchTarget_Left/Right |
| `Player_Status_*` | 玩家状态 | Blocking, Rage_Full, Rage_None |
| `Enemy_Ability_*` | 敌人能力 | Melee, Ranged, Summon |
| `Enemy_Status_*` | 敌人状态 | Unblockable |
| `Shared_Event_*` | 共享事件 | MeleeHit, HitReact |
| `Shared_Status_*` | 共享状态 | Dead |
| `Shared_SetByCaller_*` | 伤害参数 | BaseDamage, LightAttackCombo, HeavyAttackCombo |
| `GameData_*` | 游戏数据 | Level, SaveGame_Slot |

### 3.5 枚举与结构体 (`WarriorTypes/`)

| 类型 | 说明 |
|---|---|
| `EWarriorGameDifficulty` | 游戏难度：Easy, Normal, Hard, VeryHard |
| `EWarriorInputMode` | 输入模式：GameOnly, UIOnly |
| `EWarriorAbilityActivationPolicy` | 能力激活策略：OnTriggered（手动）, OnGiven（自动） |
| `EWarriorSurvivalGameModeState` | 生存模式状态机枚举 |
| `EToggleDamageType` | 伤害碰撞类型：CurrentEquippedWeapon, LeftHand, RightHand |
| `FWarriorHeroAbilitySet` | 英雄能力集：InputTag + AbilityToGrant |
| `FWarriorHeroSpecialAbilitySet` | 扩展英雄能力集：额外含 Icon、CooldownTag |
| `FWarriorHeroWeaponData` | 武器数据：AnimLayer、InputMapping、技能、伤害曲线、Icon |
| `FWarriorCountDownAction` | 自定义延迟倒计时 Action |

---

## 4. 角色系统

### 4.1 继承层次

```
ACharacter
  └─ AWarriorBaseCharacter               # 基类：装配 ASC + AttributeSet + MotionWarping
       ├─ AWarriorHeroCharacter          # 玩家角色
       └─ AWarriorEnemyCharacter         # 敌人角色
```

### 4.2 `AWarriorBaseCharacter`

实现三个关键接口：

| 接口 | 用途 |
|---|---|
| `IAbilitySystemInterface` | 提供 ASC 访问 |
| `IPawnCombatInterface` | 提供战斗组件访问 |
| `IPawnUIInterface` | 提供 UI 组件访问 |

| 核心成员 | 说明 |
|---|---|
| `WarriorAbilitySystemComponent` | 自定义 ASC |
| `WarriorAttributeSet` | 属性集 |
| `MotionWarpingComponent` | 运动扭曲 |
| `CharacterStartUpData` | 启动数据资产（软引用，子类多态加载） |

### 4.3 `AWarriorHeroCharacter` → `AWarriorBaseCharacter`

| 核心成员 | 说明 |
|---|---|
| `CameraBoom` | SpringArm，200 臂长 |
| `FollowCamera` | 跟随相机 |
| `HeroCombatComponent` | 英雄战斗组件 |
| `HeroUIComponent` | 英雄 UI 组件 |
| `InputConfigDataAsset` | Enhanced Input 配置资产 |

**输入绑定与处理**：

| 输入 | 动作 |
|---|---|
| Input_Move | 转换 2D 输入为世界空间移动 |
| Input_Look | 相机旋转 |
| Input_SwitchTarget | 通过 GAS 事件发送左右切换信号 |
| Input_PickUp_Stones | 通过 GAS 事件触发拾取 |
| 技能输入 (Pressed/Released) | 委托给 ASC 的 `OnAbilityInputPressed/Released` |

**难度缩放**（能力等级）：
| 难度 | 英雄等级 | 敌人等级 |
|---|---|---|
| Easy | 4 | 1 |
| Normal | 3 | 2 |
| Hard | 2 | 3 |
| VeryHard | 1 | 4 |

### 4.4 `AWarriorEnemyCharacter` → `AWarriorBaseCharacter`

| 核心成员 | 说明 |
|---|---|
| `EnemyCombatComponent` | 敌人战斗组件 |
| `EnemyUIComponent` | 敌人 UI 组件（管理血条等 Widget） |
| `EnemyHealthWidgetComponent` | 头顶血条 Widget 组件 |
| `LeftHandCollisionBox` | 左手碰撞盒（徒手攻击） |
| `RightHandCollisionBox` | 右手碰撞盒（徒手攻击） |

---

## 5. 能力系统 (GAS)

### 5.1 `UWarriorAbilitySystemComponent` → `UAbilitySystemComponent`

扩展 GAS 的核心 ASC，关键功能：

| 功能 | 说明 |
|---|---|
| `OnAbilityInputPressed(Tag)` | **Toggleable** 标签：已激活则取消，否则激活；其他：直接激活 |
| `OnAbilityInputReleased(Tag)` | **MustBeHeld** 标签：释放时取消对应能力 |
| `GrantHeroWeaponAbilities()` | 授予武器技能，将 InputTag 注入 DynamicSpecSourceTags |
| `RemoveGrantedHeroWeaponAbilities()` | 移除武器技能 |
| `TryActivateAbilityByTag(Tag)` | 按 Tag 随机选择一个匹配能力激活 |

### 5.2 `UWarriorAttributeSet` → `UAttributeSet`

七项核心属性，均设置 `ATTRIBUTE_ACCESSORS` 宏：

| 属性 | 说明 |
|---|---|
| `CurrentHealth` / `MaxHealth` | 生命值（当前/上限） |
| `CurrentRage` / `MaxRage` | 怒气值（当前/上限） |
| `AttackPower` | 攻击力 |
| `DefensePower` | 防御力 |
| `DamageTaken` | 本次承受伤害量 |

**`PostGameplayEffectExecute` 核心逻辑**：

1. **生命处理**：`CurrentHealth = Clamp(0, MaxHealth)`，广播 `OnCurrentHealthChanged` 百分比。
2. **怒气处理**：满怒添加 `Player_Status_Rage_Full`，零怒添加 `Player_Status_Rage_None`，互斥替换，广播百分比。
3. **伤害处理**：扣除 DamageTaken 得到新生命，广播变化。生命归零添加 `Shared_Status_Dead`。

### 5.3 GameplayAbility 继承层次

```
UGameplayAbility
  └─ UWarriorGameplayAbility                   # 基础能力：激活策略、辅助函数
       ├─ UWarriorHeroGameplayAbility          # 英雄能力：英雄专属辅助 + 伤害构造
       │    ├─ HeroGameplayAbility_TargetLock  # 目标锁定
       │    └─ HeroGameplayAbility_PickUpStones # 拾取石头
       └─ UWarriorEnemyGameplayAbility         # 敌人能力：可伸缩伤害构造
```

### 5.4 `UWarriorGameplayAbility` → `UGameplayAbility`

| 关键属性/方法 | 说明 |
|---|---|
| `AbilityActivationPolicy` | OnTriggered（手动触发）/ OnGiven（获取时自动激活） |
| `GetPawnCombatComponentFromActorInfo()` | 获取战斗组件 |
| `GetWarriorAbilitySystemComponentFromActorInfo()` | 获取 Warrior ASC |
| `BP_ApplyEffectSpecHandleToTarget()` | 蓝图：向目标应用 GE |
| `ApplyGameplayEffectSpecHandleToHitResult()` | 向 HitResult 列表中的敌对 Pawn 应用伤害 |

### 5.5 `UWarriorHeroGameplayAbility` → `UWarriorGameplayAbility`

| 关键方法 | 说明 |
|---|---|
| `GetHeroCharacter/Controller/CombatComponent/UIComponentFromActorInfo()` | 懒缓存访问器 |
| `MakeHeroDamageEffectSpecHandle(EffectClass, BaseDamage, AttackTypeTag, ComboCount)` | 创建带 SetByCaller 的伤害 GE 句柄 |
| `GetAbilityRemainingCooldownByTag(Tag)` | 查询冷却剩余时间 |

### 5.6 `UWarriorEnemyGameplayAbility` → `UWarriorGameplayAbility`

| 关键方法 | 说明 |
|---|---|
| `GetEnemyCharacter/CombatComponentFromActorInfo()` | 敌人专属访问器 |
| `MakeEnemyDamageEffectSpecHandle(EffectClass, ScalableFloat)` | 使用可伸缩浮点创建伤害 GE |

### 5.7 自定义 Ability 详解

#### `HeroGameplayAbility_TargetLock`

实现类魂系目标锁定：

1. **发现目标**：前方 BoxTrace，收集唯一 Actor。
2. **锁定最近目标**：`UGameplayStatics::FindNearestActor`。
3. **锁定 Widget**：屏幕空间定位锁定图标。
4. **移动切换**：降低移动速度，添加锁定输入映射（优先级 3）。
5. **相机插值**：`FMath::RInterpTo` 平滑朝向锁定目标，支持俯仰偏移。
6. **左右切换**：Cross Product（Z 分量）区分左右，选择最近目标。

#### `HeroGameplayAbility_PickUpStones`

两阶段拾取：
1. **CollectStones()**：BoxTrace 下方，收集范围内 `AWarriorStoneBase`。
2. **ConsumeStones()**：对收集的石头逐一调用 `Consume(ASC, Level)`，施加 GE。

### 5.8 Ability Tasks

| 任务 | 说明 |
|---|---|
| `AbilityTask_ExecuteTaskOnTick` | 每帧广播 DeltaTime，供蓝图能力驱动 Tick 逻辑 |
| `AbilityTask_WaitSpawnEnemies` | 等待 GameplayEvent 后，异步加载并生成敌人，支持随机半径 |

### 5.9 `UGEExecCalc_DamageTaken` — 伤害执行计算

**公式**：`最终伤害 = BaseDamage × 源攻击力 ÷ 目标防御力`

**捕获属性**：Source:AttackPower、Target:DefensePower、Target:DamageTaken

**SetByCaller 参数**：
| Tag | 用途 |
|---|---|
| `Shared_SetByCaller_BaseDamage` | 基础伤害值 |
| `UsedLightAttackComboCount` | 轻攻击连击数（1-4） |
| `UsedHeavyAttackComboCount` | 重攻击连击数（1-2） |

**连击倍率**：

| 连击类型 | 公式 | 最大倍率 |
|---|---|---|
| 轻攻击 | `(ComboCount - 1) × 0.05 + 1.0` | 1.15x |
| 重攻击 | `ComboCount × 0.15 + 1.0` | 1.60x |

---

## 6. 战斗系统

### 6.1 `UPawnCombatComponent` → `UPawnExtensionComponentBase`

核心战斗组件基类：

| 功能 | 说明 |
|---|---|
| `RegisterSpawnedWeapon(Tag, Weapon, Equip)` | 武器注册，绑定 Hit/Pull 委托 |
| `GetCharacterCarriedWeaponByTag(Tag)` | 按 Tag 获取武器 |
| `GetCharacterCurrentEquippedWeapon()` | 获取当前装备武器 |
| `ToggleWeaponCollision(Enabled, ToggleType)` | 切换武器或身体碰撞 |

### 6.2 `UHeroCombatComponent` → `UPawnCombatComponent`

| 重写逻辑 | 说明 |
|---|---|
| `OnHitTargetActor()` | 去重后发送 `Shared_Event_MeleeHit` + `Player_Event_HitPause`（打击停顿） |
| `OnWeaponPulledFromTargetActor()` | 发送 `Player_Event_HitPause`（Hitstop 反馈） |

### 6.3 `UEnemyCombatComponent` → `UPawnCombatComponent`

| 重写逻辑 | 说明 |
|---|---|
| `OnHitTargetActor()` | 检查玩家是否格挡 + 攻击是否不可格挡。有效格挡发送 `Player_Event_SuccessfulBlock`，否则发送 `Shared_Event_MeleeHit` |
| `ToggleBodyCollisionBoxCollision()` | 按左手/右手独立开关碰撞盒 |

### 6.4 武器系统

```
AActor
  └─ AWarriorWeaponBase              # 武器基类：Mesh + 碰撞盒 + 委托
       └─ AWarriorHeroWeapon         # 英雄武器：附带 FWarriorHeroWeaponData
```

| 委托 | 触发时机 |
|---|---|
| `OnWeaponHitTarget` | 碰撞盒与敌对 Pawn 重叠 |
| `OnWeaponPulledFromTarget` | 碰撞盒与敌对 Pawn 结束重叠 |

### 6.5 弹射物系统

`AWarriorProjectileBase`：

| 属性 | 说明 |
|---|---|
| `ProjectileCollisionBox` | 碰撞盒 |
| `ProjectileNiagaraComponent` | Niagara 粒子特效 |
| `ProjectileMovementComponent` | 弹射运动（初速 400，无重力，5 秒生命） |
| `ProjectileDamagePolicy` | OnHit（命中） / OnBeginOverlap（重叠） |

**命中逻辑**：检测格挡（玩家有 `Player_Status_Blocking` + 点积验证），有效格挡发送 `Player_Event_SuccessfulBlock`，否则应用伤害 + 发送 `Shared_Event_HitReact`。

#### 6.5.1 魔法飞弹 (`ASpellbladeMagicMissile`)

继承自 `AWarriorProjectileBase`，实现链式弹射追踪：

| 属性 | 说明 |
|---|---|
| `MaxOverlapNums` | 最大链式命中次数（默认 6） |
| `CurrentOverlapNums` | 当前已命中次数（BlueprintReadOnly，供 UI 绑定） |
| `BoxTraceDistance` / `TraceBoxSize` | BoxTrace 扫描参数 |
| `CurrentFaceActor` | 当前追踪目标（`TWeakObjectPtr`） |
| `RotationInterpSpeed` | 转向插值速度 |
| `InstigatorGameplayEffectClass` | 命中时对施法者施加的 GE（如治疗 / 怒气回复） |
| `AbilityLevel` | 从生成技能传入的等级（`ExposeOnSpawn`），用于 GE 等级缩放 |
| `HitTargets` | 已命中目标去重数组，避免同一敌人被反复锁定 |
| `bShouldFaceToTarget` | 是否持续追踪目标 |

**核心逻辑**：

1. **生成时扫描**（`BeginPlay`）：`BoxTraceMultiForObjects` 搜索范围内敌对目标，`FindNearestActor` 锁定最近目标
2. **每帧追踪**（`Tick`）：`FMath::RInterpTo` 平滑转向目标，同步更新 `ProjectileMovementComponent->Velocity` 保持速度方向与朝向一致
3. **命中时链式跳转**（`OnProjectileBeginOverlap`）：施加伤害 → 播放命中特效 → 目标加入 `HitTargets` 去重 → 重新 BoxTrace 扫描剩余目标 → 锁定下一个最近目标（排除已死亡和已命中者）
4. **施法者回馈**（`HandleApplyInstigatorGameplayEffect`）：每次命中时对 `GetInstigator()` 施加 `InstigatorGameplayEffectClass`，用于实现命中回血、怒气回复等
5. **链尽处理**：当 `AvailableActorsToFly` 为空或达到 `MaxOverlapNums` 时，`bShouldFaceToTarget = false`，弹射物直线飞出或超时自毁

### 6.6 格挡机制

`UWarriorBlueprintFunctionLibrary::IsValidBlock()`：检查攻击者与防御者的前向点积 < -0.6（双方朝向夹角 < ~53°）。

### 6.7 HitReact 方向计算

`ComputeHitReactDirectionTag()`：根据受害者前向与伤害来源方向的夹角 + Cross Product 符号，返回 Front / Left / Right / Back Tag。

---

## 7. UI 系统

### 7.1 组件继承层次

```
UActorComponent
  └─ UPawnExtensionComponentBase
       └─ UPawnUIComponent                      # 基类：生命百分比委托
            ├─ UHeroUIComponent                 # 英雄：怒气、武器、技能冷却、拾取
            └─ UEnemyUIComponent                # 敌人：绑定 Widget 管理
```

### 7.2 UHeroUIComponent 委托一览

| 委托 | 说明 |
|---|---|
| `OnCurrentHealthChanged` | 生命百分比变化 → HUD |
| `OnCurrentRageChanged` | 怒气百分比变化 → HUD |
| `OnEquippedWeaponChanged` | 武器图标更新 |
| `OnAbilityIconSlotUpdated` | 技能图标槽更新 |
| `OnAbilityCooldownBegin` | 技能冷却开始通知 |
| `OnStoneInteracted` | 拾取交互提示显示/隐藏 |

### 7.3 UEnemyUIComponent

管理敌人身上绘制的 Widget（血条等）：
- `RegisterEnemyDrawnWidget()` — 注册
- `RemoveEnemyDrawnWidgetsIfAny()` — 移除全部

### 7.4 `UWarriorWidgetBase` → `UUserWidget`

| BlueprintImplementableEvent | 说明 |
|---|---|
| `BP_OnOwningHeroUIComponentInitialized` | 英雄 UI 组件初始化回调 |
| `BP_OnOwningEnemyUIComponentInitialized` | 敌人 UI 组件初始化回调 |

---

## 8. AI 系统

### 8.1 `AWarriorAIController` → `AAIController`

| 特性 | 配置 |
|---|---|
| Team ID | 1（玩家 = Team 0） |
| 寻路 | `UCrowdFollowingComponent`（人群避让） |
| AI 感知 | 视觉：5000 半径，180° 视野 |
| 避让质量 | 1-4 可配 |
| 碰撞查询 | 可配范围 |

感知到目标后自动写入 Blackboard 的 `TargetActor` Key。

### 8.2 行为树节点

| 类型 | 节点 | 功能 |
|---|---|---|
| Task | `BTTask_RotateToFaceTarget` | Tick 插值旋转朝向目标，点积验证角度精度 |
| Decorator | `BTDecorator_ShouldAbortAllLogic` | 目标死亡/自身死亡/距离归零 → 中断当前 BT 分支 |
| Decorator | `BTDecorator_DoesActorHasTag` | 检查目标是否有指定 GameplayTag |
| Service | `BTService_OrientToTargetActor` | 每帧插值朝向目标 |
| Service | `BTService_GetDistToTarget` | 每 0.2s 计算并写入与目标距离 |

### 8.3 团队敌对判定

`IGenericTeamAgentInterface`：Team ID 不同即为敌对。

---

## 9. 动画系统

### 9.1 继承层次

```
UAnimInstance
  └─ UWarriorBaseAnimInstance                 # 基类：ThreadSafe GameplayTag 检查
       ├─ UWarriorCharacterAnimInstance       # 角色：移动参数（速度/加速度/方向）
       │    └─ UWarriorHeroAnimInstance       # 英雄：Idle → Relax 状态转换
       └─ UWarriorHeroLinkedAnimLayer         # 英雄链接动画层
```

### 9.2 关键参数

| AnimInstance | 参数 | 说明 |
|---|---|---|
| `WarriorCharacterAnimInstance` | `GroundSpeed` | 2D 速度大小 |
| | `bHasAcceleration` | 是否有加速度（Squared 优化） |
| | `LocomotionDirection` | 基于 `UKismetAnimationLibrary::CalculateDirection` |
| `WarriorHeroAnimInstance` | `bShouldEnterRelaxState` | 空闲超过阈值进入放松状态 |

---

## 10. 数据资产

### 10.1 `UDataAsset_InputConfig` → `UDataAsset`

| 成员 | 说明 |
|---|---|
| `DefaultMappingContext` | 默认输入映射上下文 |
| `NativeInputActions` | 原生输入（移动、镜头） |
| `AbilityInputActions` | 技能输入（攻击、翻滚等） |

### 10.2 StartUpData 数据资产

```
UDataAsset_StartUpDataBase           # 基类：激活技能 + 被动技能 + 启动 GE
  ├─ UDataAsset_HeroStartUpData     # 英雄：附加 HeroAbilitySets（InputTag → Ability）
  └─ UDataAsset_EnemyStartUpData    # 敌人：附加 EnemyCombatAbilities
```

启动时通过 `GiveToAbilitySystemComponent(ASC, Level)` 统一授予能力和应用效果。

---

## 11. 拾取物系统

```
AActor
  └─ AWarriorPickUpBase              # 基础拾取物：球形碰撞检测
       └─ AWarriorStoneBase          # 石头：碰撞时激活 PickUp 技能，消耗时施加 GE
```

碰撞后触发 GAS 事件激活拾取能力，收集后统一消耗。

---

## 12. 存档系统

`UWarriorSaveGame` → `USaveGame`：仅存储 `EWarriorGameDifficulty`，通过 `SaveCurrentGameDifficulty()` / `TryLoadSavedGameDifficulty()` 读写。

---

## 13. 蓝图工具函数库 (`UWarriorBlueprintFunctionLibrary`)

| 函数 | 功能 |
|---|---|
| `Add/RemoveGameplayTagToActorIfNone/Found` | 标签管理 |
| `BP_DoesActorHasTag` | Tag 检测（返回 ConfirmType） |
| `BP_GetPawnCombatComponent` | 获取战斗组件（返回 ValidType） |
| `IsTargetPawnHostile` | 团队敌对检查 |
| `GetScalableFloatValueAtLevel` | 可伸缩浮点读取 |
| `ComputeHitReactDirectionTag` | 受击方向 Tag 计算 |
| `IsValidBlock` | 格挡有效性验证 |
| `ApplyGameplayEffectSpecHandleToTargetActor` | 为目标应用 GE |
| `CountDown` | 蓝图延迟倒计时 |
| `GetWarriorGameInstance` | 获取 GameInstance |
| `ToggleInputMode` | 切换 GameOnly / UIOnly 输入模式 |
| `SaveCurrentGameDifficulty / TryLoadSavedGameDifficulty` | 难度存档读写 |

---

## 14. 核心数据流

### 14.1 伤害流程

```
武器碰撞重叠 / 弹射物命中
    │
    ├─ 检查敌对关系 (IsTargetPawnHostile)
    ├─ 检查格挡 (Player_Status_Blocking + IsValidBlock)
    │    ├─ 有效格挡 → Player_Event_SuccessfulBlock
    │    └─ 不可格挡 / 无效格挡 ↓
    └─ MakeDamageEffectSpecHandle (BaseDamage + ComboCount + AttackType)
         │
         └─ GEExecCalc_DamageTaken
              │
              ├─ 捕获 Source.AttackPower, Target.DefensePower
              ├─ 提取 SetByCaller: BaseDamage, ComboCount
              ├─ 计算：BaseDamage × AttackPower / DefensePower × ComboScalar
              └─ 输出到 Target.DamageTaken
                   │
                   └─ PostGameplayEffectExecute → 扣血 → Clamp → 广播 UI
```

### 14.2 输入 → 能力流程

```
Enhanced Input Action (Pressed/Released)
    │
    └─ UWarriorEnhancedInputComponent::BindAbilityInputAction
         │
         └─ ASC::OnAbilityInputPressed/Released(Tag)
              │
              ├─ 匹配 DynamicSpecSourceTags
              ├─ Toggleable → 切换激活
              ├─ MustBeHeld → 释放时取消
              └─ 普通 → 激活能力
```

### 14.3 UI 更新链路

```
AttributeSet::PostGameplayEffectExecute
    │
    └─ UIComponent 委托 (OnCurrentHealthChanged / OnCurrentRageChanged)
         │
         └─ UWarriorWidgetBase::BP_OnOwningXxxUIComponentInitialized
              │
              └─ Blueprint Layout 响应更新
```

---

## 15. 关键设计模式

| 模式 | 应用场景 |
|---|---|
| **接口分离** | `IPawnCombatInterface` / `IPawnUIInterface` — 解耦角色与组件访问 |
| **数据驱动** | DataAsset 配置输入、启动数据；DataTable 驱动波次生成 |
| **委托/事件** | UI 组件 ↔ Widget 通信；武器碰撞 ↔ 战斗组件响应；GameMode 状态机 ↔ HUD |
| **桥接模式** | `WarriorBlueprintFunctionLibrary` 作为 C++ ↔ 蓝图桥梁 |
| **懒加载** | Ability 中 `TWeakObjectPtr` 缓存 ActorInfo 衍生引用 |
| **Tag 驱动架构** | 输入映射 → 能力激活 → 状态管理 → 动画切换，全程 GameplayTag |
