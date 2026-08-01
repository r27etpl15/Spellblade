// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "WarriorBlueprintFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "Interfaces/PawnUIInterface.h"
#include "Components/UI/PawnUIComponent.h"
#include "Components/UI/HeroUIComponent.h"

#include "WarriorDebugHelper.h"

UWarriorAttributeSet::UWarriorAttributeSet()
{
	InitCurrentHealth(1.0f);
	InitMaxHealth(1.0f);
	InitCurrentRage(1.0f);
	InitMaxRage(1.0f);
	InitAttackPower(1.0f);
	InitDefensePower(1.0f);
	
}

void UWarriorAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if (!CachedPawnUIInterface.IsValid())
	{
		CachedPawnUIInterface = TWeakInterfacePtr<IPawnUIInterface>(Data.Target.GetAvatarActor());
	}
	
	checkf(CachedPawnUIInterface.IsValid(), TEXT("%s 没有实现IPawnUIInterface接口"), 
		*Data.Target.GetAvatarActor()->GetActorNameOrLabel());
	
	UPawnUIComponent* PawnUIComponent = CachedPawnUIInterface->GetPawnUIComponent();
	
	checkf(PawnUIComponent, TEXT("无法从%s中提取PawnUIComponent"), *Data.Target.GetAvatarActor()->GetActorNameOrLabel());
	
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		const float NewCurrentHealth = FMath::Clamp(GetCurrentHealth(), 0.0f, GetMaxHealth());
		
		SetCurrentHealth(NewCurrentHealth);
		
		PawnUIComponent->OnCurrentHealthChanged.Broadcast(GetCurrentHealth() / GetMaxHealth());
	}
	
	if (Data.EvaluatedData.Attribute == GetCurrentRageAttribute())
	{
		const float NewCurrentRage = FMath::Clamp(GetCurrentRage(), 0.0f, GetMaxRage());
		
		SetCurrentRage(NewCurrentRage);
		
		if (GetCurrentRage() == GetMaxRage())
		{
			UWarriorBlueprintFunctionLibrary::AddGameplayTagToActorIfNone(Data.Target.GetAvatarActor(),
				WarriorGameplayTags::Player_Status_Rage_Full);
		}
		else if (GetCurrentRage() == 0.0f)
		{
			UWarriorBlueprintFunctionLibrary::AddGameplayTagToActorIfNone(Data.Target.GetAvatarActor(),
				WarriorGameplayTags::Player_Status_Rage_None);
		}
		else
		{
			UWarriorBlueprintFunctionLibrary::RemoveGameplayTagFromActorIfFound(Data.Target.GetAvatarActor(),
				WarriorGameplayTags::Player_Status_Rage_Full);
			UWarriorBlueprintFunctionLibrary::RemoveGameplayTagFromActorIfFound(Data.Target.GetAvatarActor(),
				WarriorGameplayTags::Player_Status_Rage_None);
		}
		
		if (UHeroUIComponent* HeroUIComponent = CachedPawnUIInterface->GetHeroUIComponent())
		{
			HeroUIComponent->OnCurrentRageChanged.Broadcast(GetCurrentRage() / GetMaxRage());
		}
	}
	
	if (Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
	{
		const float OldHealth = GetCurrentHealth();
		const float DamageDone = GetDamageTaken();
		
		const float NewCurrentHealth = FMath::Clamp(OldHealth - DamageDone, 0.0f, GetMaxHealth());
		
		SetCurrentHealth(NewCurrentHealth);
		
		// Debug::Print(FString::Printf(TEXT("原本的生命值: %f, 新的生命值: %f"), OldHealth, NewCurrentHealth));
		
		PawnUIComponent->OnCurrentHealthChanged.Broadcast(GetCurrentHealth() / GetMaxHealth());
		
		// 处理角色死亡
		if (GetCurrentHealth() == 0.0f)
		{
			UWarriorBlueprintFunctionLibrary::AddGameplayTagToActorIfNone(Data.Target.GetAvatarActor(), 
				WarriorGameplayTags::Shared_Status_Dead);
			
		}
	}
}
