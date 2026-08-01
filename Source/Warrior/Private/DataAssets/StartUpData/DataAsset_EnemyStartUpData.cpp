// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/StartUpData/DataAsset_EnemyStartUpData.h"

#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"

void UDataAsset_EnemyStartUpData::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive,
	const int32 ApplyLevel)
{
	Super::GiveToAbilitySystemComponent(InAscToGive, ApplyLevel);
	
	 if (!EnemyCombatAbilities.IsEmpty())
	 {
		 for (const TSubclassOf<UWarriorEnemyGameplayAbility> AbilityClass : EnemyCombatAbilities)
		 {
			if (!AbilityClass)
			{
				continue;
			}
		 	
		 	FGameplayAbilitySpec AbilitySpe(AbilityClass);
		 	AbilitySpe.SourceObject = InAscToGive->GetAvatarActor();
		 	AbilitySpe.Level = ApplyLevel;
		 	
		 	InAscToGive->GiveAbility(AbilitySpe);
		 }
	 }
}
