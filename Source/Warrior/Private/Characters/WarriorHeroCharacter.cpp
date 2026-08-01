// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorHeroCharacter.h"
#include "WarriorGameplayTags.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/Combat/HeroCombatComponent.h"
#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Components/Input/WarriorEnhancedInputComponent.h"
#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include "Components/UI/HeroUIComponent.h"
#include "EnhancedInputSubsystemInterface.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "WarriorDebugHelper.h"
#include "GameModes/WarriorBaseGameMode.h"

AWarriorHeroCharacter::AWarriorHeroCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// 控制器旋转时，不要让角色跟着旋转，让旋转只影响摄像机
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 55.0f, 65.0f);
	// 弹簧臂的旋转将不再使用它自身在角色上的相对旋转，而是直接使用 Pawn 的控制旋转
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 角色面朝移动方向
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	
	HeroCombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("HeroCombatComponent"));
	
	HeroUIComponent = CreateDefaultSubobject<UHeroUIComponent>(TEXT("HeroUIComponent"));
}

UPawnCombatComponent* AWarriorHeroCharacter::GetPawnCombatComponent() const
{
	return HeroCombatComponent;
}

UPawnUIComponent* AWarriorHeroCharacter::GetPawnUIComponent() const
{
	return HeroUIComponent;
}

void AWarriorHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (!CharacterStartUpData.IsNull())
	{
		// 因为技能配置文件比较小，所以使用同步加载就足够了
		if (UDataAsset_StartUpDataBase* LoadedData = CharacterStartUpData.LoadSynchronous())
		{
			int32 AbilityApplyLevel = 1;
			
			if (AWarriorBaseGameMode* BaseGameMode = GetWorld()->GetAuthGameMode<AWarriorBaseGameMode>())
			{
				switch (BaseGameMode->GetCurrentGameDifficulty())
				{
				case EWarriorGameDifficulty::Easy:
					AbilityApplyLevel = 4;
					Debug::Print(TEXT("当前难度：简单"));
					break;
				case EWarriorGameDifficulty::Normal:
					AbilityApplyLevel = 3;
					Debug::Print(TEXT("当前难度：普通"));
					break;
				case EWarriorGameDifficulty::Hard:
					AbilityApplyLevel = 2;
					Debug::Print(TEXT("当前难度：困难"));
					break;
				case EWarriorGameDifficulty::VeryHard:
					AbilityApplyLevel = 1;
					Debug::Print(TEXT("当前难度：地狱"));
					break;
				}
			}
			
			LoadedData->GiveToAbilitySystemComponent(WarriorAbilitySystemComponent, AbilityApplyLevel);
		}
	}
}

void AWarriorHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (WarriorAbilitySystemComponent && WarriorAttributeSet)
	{
		// const FString ASCText = FString::Printf(TEXT("Owner Actor: %s, Avatar Actor: %s"),
		// 	*WarriorAbilitySystemComponent->GetOwnerActor()->GetActorLabel(),
		// 	*WarriorAbilitySystemComponent->GetAvatarActor()->GetActorLabel());
		// Debug::Print(TEXT("能力系统组件不为空") + ASCText, FColor::Blue);
	}
}

void AWarriorHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	checkf(InputConfigDataAsset, TEXT("Forgot to assign a valid data asset as input config"));

	// 绑定输入映射上下文
	ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	check(Subsystem);
	
	// 优先级设置为0， 为了装备武器后重写输入映射上下文
	Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);

	// 绑定输入操作
	UWarriorEnhancedInputComponent* WarriorEnhancedInputComponent = 
		Cast<UWarriorEnhancedInputComponent>(PlayerInputComponent);
	if (WarriorEnhancedInputComponent)
	{
		WarriorEnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, WarriorGameplayTags::InputTag_Move,
			ETriggerEvent::Triggered, this, &ThisClass::InputMove);
		WarriorEnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, WarriorGameplayTags::InputTag_Look,
			ETriggerEvent::Triggered, this, &ThisClass::InputLook);
		
		WarriorEnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, 
			WarriorGameplayTags::InputTag_SwitchTarget, ETriggerEvent::Triggered, this, 
			&ThisClass::Input_SwitchTargetTriggered);
		WarriorEnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, 
			WarriorGameplayTags::InputTag_SwitchTarget, ETriggerEvent::Completed, this, 
			&ThisClass::Input_SwitchTargetCompleted);
		
		WarriorEnhancedInputComponent->BindNativeInputAction(InputConfigDataAsset, 
			WarriorGameplayTags::InputTag_PickUp_Stones, ETriggerEvent::Started, this,
			&ThisClass::Input_PickUpStonesStarted);
		
		WarriorEnhancedInputComponent->BindAbilityInputAction(InputConfigDataAsset, this, 
			&ThisClass::Input_AbilityInputPressed, &ThisClass::Input_AbilityInputReleased);
	}
}

void AWarriorHeroCharacter::InputMove(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	const FRotator MovementRotation(0.0, Controller->GetControlRotation().Yaw, 0.0);
	
	if (MovementVector.Y != 0.0)
	{
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);

		AddMovementInput(ForwardDirection, MovementVector.Y);
	}

	if (MovementVector.X != 0)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);

		AddMovementInput(RightDirection, MovementVector.X);
				
	}
}

void AWarriorHeroCharacter::InputLook(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if (LookAxisVector.X != 0.0)
	{
		AddControllerYawInput(LookAxisVector.X);
	}

	if (LookAxisVector.Y != 0)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AWarriorHeroCharacter::Input_SwitchTargetTriggered(const FInputActionValue& InputActionValue)
{
	SwitchDirection = InputActionValue.Get<FVector2D>();
}

void AWarriorHeroCharacter::Input_SwitchTargetCompleted(const FInputActionValue& InputActionValue)
{
	FGameplayTag SwitchDirectionTag= SwitchDirection.X > 0.0f ? 
		WarriorGameplayTags::Player_Event_SwitchTarget_Right : WarriorGameplayTags::Player_Event_SwitchTarget_Left;
	
	FGameplayEventData Data;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this, 
		SwitchDirectionTag,
		Data
	);
}

void AWarriorHeroCharacter::Input_PickUpStonesStarted(const FInputActionValue& InputActionValue)
{
	FGameplayEventData Data;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		WarriorGameplayTags::Player_Event_ConsumesStones,
		Data
	);
}

void AWarriorHeroCharacter::Input_AbilityInputPressed(FGameplayTag InInputTag)
{
	// GAS中不通过回调函数调用技能，而是交给ASC去调用
	WarriorAbilitySystemComponent->OnAbilityInputPressed(InInputTag);
}

void AWarriorHeroCharacter::Input_AbilityInputReleased(FGameplayTag InInputTag)
{
	// GAS中不通过回调函数调用技能，而是交给ASC去调用
	WarriorAbilitySystemComponent->OnAbilityInputReleased(InInputTag);
}
