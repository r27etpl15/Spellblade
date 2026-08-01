// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WarriorTypes/WarriorEnumTypes.h"
#include "WarriorBlueprintFunctionLibrary.generated.h"

struct FGameplayTag;
struct FScalableFloat;

class UWarriorAbilitySystemComponent;
class UPawnCombatComponent;
class UWarriorGameInstance;

/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static UWarriorAbilitySystemComponent* NativeGetWarriorASCFromActor(AActor* InActor);
	static bool NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary")
	static void AddGameplayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary")
	static void RemoveGameplayTagFromActorIfFound(AActor* InActor, FGameplayTag TagToRemove);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary", meta=(DisplayName = "Does Actor Has Tag", 
		ExpandEnumAsExecs = "OutConfirmType"))
	static void BP_DoesActorHasTag(AActor* InActor, FGameplayTag TagToCheck, EWarriorConfirmType& OutConfirmType); 
	
	static UPawnCombatComponent* NativeGetPawnCombatComponent(AActor* InActor);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary", meta=(DisplayName = "Get Pawn Combat Component From Actor", 
		ExpandEnumAsExecs = "OutValidType"))
	static UPawnCombatComponent* BP_GetPawnCombatComponent(AActor* InActor, EWarriorValidType& OutValidType);
	
	UFUNCTION(BlueprintPure, Category="Warrior|FunctionLibrary")
	static bool IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn);
	
	UFUNCTION(BlueprintPure, Category="Warrior|FunctionLibrary", meta=(CompactNodeTitle = "Get Value At Level"))
	static float GetScalableFloatValueAtLevel(const FScalableFloat& InScalableFloat, const float InLevel = 1.0f);
	
	UFUNCTION(BlueprintPure, Category="Warrior|FunctionLibrary")
	static FGameplayTag ComputeHitReactDirectionTag(AActor* InAttacter, AActor* InVictim, float& OutAngleDifference);
	
	UFUNCTION(BlueprintPure, Category="Warrior|FunctionLibrary")
	static bool IsValidBlock(AActor* InAttacter, AActor* InDefender);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary")
	static bool ApplyGameplayEffectSpecHandleToTargetActor(AActor* InInstigator, AActor* InTargetActor, 
		const FGameplayEffectSpecHandle& InSpecHandle);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary", meta=(Latent, WorldContext = "WorldContextObject",
		LatentInfo = "LatentInfo", ExpandEnumAsExecs = "CountDownInput, CountDownOutput", TotalTime = "1.0",
		UpdateInterval = "0.0"))
	static void CountDown(const UObject* WorldContextObject, float TotalTime, float UpdateInterval, float& OutRemainingTime,
		EWarriorCountDownActionInput CountDownInput, UPARAM(DisplayName = "Output") 
		EWarriorCountDownActionOutput& CountDownOutput, FLatentActionInfo LatentInfo);
	
	UFUNCTION(BlueprintPure, Category="Warrior|FunctionLibrary", meta=(WorldContext = "WorldContextObject"))
	static UWarriorGameInstance* GetWarriorGameInstance(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary", meta=(WorldContext = "WorldContextObject"))
	static void ToggleInputMode(EWarriorInputMode InInputMode, const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary")
	static void SaveCurrentGameDifficulty(EWarriorGameDifficulty InDifficultyToSave);
	
	UFUNCTION(BlueprintCallable, Category="Warrior|FunctionLibrary")
	static bool TryLoadSavedGameDifficulty(EWarriorGameDifficulty& OutSavedDifficulty);
};
