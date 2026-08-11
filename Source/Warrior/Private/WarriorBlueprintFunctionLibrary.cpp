// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorBlueprintFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Interfaces/PawnCombatInterface.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "WarriorGameplayTags.h"
#include "WarriorTypes/WarriorCountDownAction.h"
#include "WarriorGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGame/WarriorSaveGame.h"

#include "WarriorDebugHelper.h"

UWarriorAbilitySystemComponent* UWarriorBlueprintFunctionLibrary::NativeGetWarriorASCFromActor(AActor* InActor)
{
	ensure(InActor);
	
	return CastChecked<UWarriorAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor));
}

void UWarriorBlueprintFunctionLibrary::AddGameplayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);
	
	if (!ASC->HasMatchingGameplayTag(TagToAdd))
	{
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

void UWarriorBlueprintFunctionLibrary::RemoveGameplayTagFromActorIfFound(AActor* InActor, FGameplayTag TagToRemove)
{
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);
	
	if (ASC->HasMatchingGameplayTag(TagToRemove))
	{
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}
}

void UWarriorBlueprintFunctionLibrary::BP_DoesActorHasTag(AActor* InActor, FGameplayTag TagToCheck,
	EWarriorConfirmType& OutConfirmType)
{
	OutConfirmType = NativeDoesActorHaveTag(InActor, TagToCheck) ? EWarriorConfirmType::Yes : EWarriorConfirmType::No;
}

UPawnCombatComponent* UWarriorBlueprintFunctionLibrary::NativeGetPawnCombatComponent(AActor* InActor)
{
	ensure(InActor);
	
	if (IPawnCombatInterface* PawnCombatInterface = Cast<IPawnCombatInterface>(InActor))
	{
		return PawnCombatInterface->GetPawnCombatComponent();
	}
	
	return nullptr;
}

UPawnCombatComponent* UWarriorBlueprintFunctionLibrary::BP_GetPawnCombatComponent(AActor* InActor,
	EWarriorValidType& OutValidType)
{
	UPawnCombatComponent* CombatComponent = NativeGetPawnCombatComponent(InActor);
	
	OutValidType = CombatComponent ? EWarriorValidType::Valid : EWarriorValidType::InValid;
	
	return CombatComponent;
}

bool UWarriorBlueprintFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	ensure(QueryPawn && TargetPawn);
	
	IGenericTeamAgentInterface* QueryTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
	IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());
	
	if (QueryTeamAgent && TargetTeamAgent)
	{
		return QueryTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
	}
	
	return false;
}

float UWarriorBlueprintFunctionLibrary::GetScalableFloatValueAtLevel(const FScalableFloat& InScalableFloat,
	float InLevel)
{
	return InScalableFloat.GetValueAtLevel(InLevel);
}

FGameplayTag UWarriorBlueprintFunctionLibrary::ComputeHitReactDirectionTag(AActor* InAttacter, AActor* InVictim,
	float& OutAngleDifference)
{
	ensure(InAttacter && InVictim);
	
	const FVector VictimForward = InVictim->GetActorForwardVector();
	const FVector VictimToAttacter = (InAttacter->GetActorLocation() - InVictim->GetActorLocation()).GetSafeNormal();
	
	OutAngleDifference = UKismetMathLibrary::DegAcos(FVector::DotProduct(VictimForward, VictimToAttacter));
	
	const FVector CrossResult = FVector::CrossProduct(VictimForward, VictimToAttacter);
	
	if (CrossResult.Z < 0.0f)
	{
		OutAngleDifference *= -1.0f;
	}
	
	if (OutAngleDifference >= -45.0f && OutAngleDifference <= 45.0f)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Front;
	}
	else if (OutAngleDifference < -45.0f && OutAngleDifference >= -135.0f)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Left;
	}
	else if (OutAngleDifference < -135.0f || OutAngleDifference > 135.0f)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Back;
	}
	else if (OutAngleDifference > 45.0f && OutAngleDifference <= 135.0f)
	{
		return WarriorGameplayTags::Shared_Status_HitReact_Right;
	}
	
	return WarriorGameplayTags::Shared_Status_HitReact_Front;
}

bool UWarriorBlueprintFunctionLibrary::IsValidBlock(AActor* InAttacter, AActor* InDefender)
{
	ensure(InAttacter && InDefender);
	
	const float DotProductResult = FVector::DotProduct(InAttacter->GetActorForwardVector(), 
		InDefender->GetActorForwardVector());
	
	return DotProductResult < -0.6f;
}

bool UWarriorBlueprintFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(AActor* InInstigator,
	AActor* InTargetActor, const FGameplayEffectSpecHandle& InSpecHandle)
{
	UWarriorAbilitySystemComponent* SourceASC = NativeGetWarriorASCFromActor(InInstigator);
	UWarriorAbilitySystemComponent* TargetASC = NativeGetWarriorASCFromActor(InTargetActor);
	
	FActiveGameplayEffectHandle ActiveGameplayEffectHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*InSpecHandle.Data, 
		TargetASC);
	
	return ActiveGameplayEffectHandle.WasSuccessfullyApplied();
}

bool UWarriorBlueprintFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck)
{
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);
	
	return ASC->HasMatchingGameplayTag(TagToCheck);
}

void UWarriorBlueprintFunctionLibrary::CountDown(const UObject* WorldContextObject, float TotalTime,
	float UpdateInterval, float& OutRemainingTime, EWarriorCountDownActionInput CountDownInput,
	UPARAM(DisplayName = "Output") EWarriorCountDownActionOutput& CountDownOutput, FLatentActionInfo LatentInfo)
{
	UWorld* World = nullptr;
	
	if (GEngine)
	{
		World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	}
	
	if (!World)
	{
		return;
	}
	
	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	
	FWarriorCountDownAction* FindAction = 
		LatentActionManager.FindExistingAction<FWarriorCountDownAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
	
	if (CountDownInput ==EWarriorCountDownActionInput::Start)
	{
		if (!FindAction)
		{
			LatentActionManager.AddNewAction(
				LatentInfo.CallbackTarget,
				LatentInfo.UUID,
				new FWarriorCountDownAction(TotalTime, UpdateInterval, OutRemainingTime, CountDownOutput, LatentInfo)
			);
		}
	}
	
	if (CountDownInput == EWarriorCountDownActionInput::Cancel)
	{
		if (FindAction)
		{
			FindAction->CancelAction();
		}
	}
}

UWarriorGameInstance* UWarriorBlueprintFunctionLibrary::GetWarriorGameInstance(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			return World->GetGameInstance<UWarriorGameInstance>();
		}
	}
	
	return nullptr;
}

void UWarriorBlueprintFunctionLibrary::ToggleInputMode(EWarriorInputMode InInputMode, const UObject* WorldContextObject)
{
	APlayerController* PlayerController = nullptr;
	
	if (GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			PlayerController = World->GetFirstPlayerController();
		}
	}
	
	if (!PlayerController)
	{
		return;
	}
	
	FInputModeGameOnly GameOnlyMode;
	FInputModeUIOnly UIOnlyMode;

	switch (InInputMode)
	{
	case EWarriorInputMode::GameOnly:
		PlayerController->SetInputMode(GameOnlyMode);
		PlayerController->bShowMouseCursor = false;
		
		break;
		
	case EWarriorInputMode::UIOnly:
		PlayerController->SetInputMode(UIOnlyMode);
		PlayerController->bShowMouseCursor = true;
		
		break;
		
	default:
		break;
	}
}

void UWarriorBlueprintFunctionLibrary::SaveCurrentGameDifficulty(EWarriorGameDifficulty InDifficultyToSave)
{
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(UWarriorSaveGame::StaticClass());
	
	if (UWarriorSaveGame* WarriorSaveGameObject = Cast<UWarriorSaveGame>(SaveGameObject))
	{
		WarriorSaveGameObject->SavedCurrentGameDifficulty = InDifficultyToSave;
		
		const bool bWasSaved = UGameplayStatics::SaveGameToSlot(WarriorSaveGameObject, 
			WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(),
			0
		);
	}
}

bool UWarriorBlueprintFunctionLibrary::TryLoadSavedGameDifficulty(EWarriorGameDifficulty& OutSavedDifficulty)
{
	if (UGameplayStatics::DoesSaveGameExist(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0))
	{
		USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(
			WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);
		
		if (UWarriorSaveGame* WarriorSaveGameObject = Cast<UWarriorSaveGame>(SaveGameObject))
		{
			OutSavedDifficulty = WarriorSaveGameObject->SavedCurrentGameDifficulty;
			
			return true;
		}
	}
	
	return false;
}

