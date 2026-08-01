// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset_StartUpDataBase.generated.h"

class UWarriorGameplayAbility;
class UWarriorAbilitySystemComponent;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class WARRIOR_API UDataAsset_StartUpDataBase : public UDataAsset
{
	GENERATED_BODY()

public:
	// 赋予角色基础技能，例如在角色背后生成斧子
	virtual void GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive, const int32 ApplyLevel = 1);
	
protected:
	// 不放在public里面，实现接口解耦
	void GrandAbilities(const TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilityToGive, 
		UWarriorAbilitySystemComponent* InAscToGive, const int32 ApplyLevel = 1);
	
	UPROPERTY(EditDefaultsOnly, Category="StartUpData")
	TArray<TSubclassOf<UWarriorGameplayAbility>> ActivateOnGivenAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category="StartUpData")
	TArray<TSubclassOf<UWarriorGameplayAbility>> ReactivateAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category="StartUpData")
	TArray<TSubclassOf<UGameplayEffect>> StartUpGameplayEffects;
};
