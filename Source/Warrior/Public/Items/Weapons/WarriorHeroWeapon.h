// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "WarriorTypes/WarriorStructTypes.h"
#include "GameplayAbilitySpecHandle.h"
#include "WarriorHeroWeapon.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorHeroWeapon : public AWarriorWeaponBase
{
	GENERATED_BODY()
	
public:
	// 用于储武器的动画层，输入映射上下文和武器的技能
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="WeaponData")
	FWarriorHeroWeaponData HeroWeaponData;
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InAbilitySpecHandles);
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<FGameplayAbilitySpecHandle> GetGrantedAbilitySpecHandles() const { return GrantedAbilitySpecHandles; }
	
private:
	// 用于清除技能
	TArray<FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;
};
