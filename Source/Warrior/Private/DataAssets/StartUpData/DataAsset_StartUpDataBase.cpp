// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"

#include "WarriorDebugHelper.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"

void UDataAsset_StartUpDataBase::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,
	const int32 ApplyLevel)
{
	check(InAscToGive);
	
	GrandAbilities(ActivateOnGivenAbilities, InAscToGive, ApplyLevel);
	GrandAbilities(ReactivateAbilities, InAscToGive, ApplyLevel);
	
	if (!StartUpGameplayEffects.IsEmpty())
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : StartUpGameplayEffects)
		{
			if (!EffectClass)
			{
				continue;
			}
			
			UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
			
			InAscToGive->ApplyGameplayEffectToSelf(
				EffectCDO,
				ApplyLevel,
				InAscToGive->MakeEffectContext()
				);
		}
	}
}

void UDataAsset_StartUpDataBase::GrandAbilities(const TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilityToGive,
	UWarriorAbilitySystemComponent* InAscToGive, const int32 ApplyLevel)
{
	if (InAbilityToGive.IsEmpty())
	{
		return;
	}
	
	for (const TSubclassOf<UWarriorGameplayAbility>& Ability : InAbilityToGive)
	{
		if (!Ability)
		{
			continue;
		}
		
		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.SourceObject = InAscToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		
		InAscToGive->GiveAbility(AbilitySpec);
	}
}
