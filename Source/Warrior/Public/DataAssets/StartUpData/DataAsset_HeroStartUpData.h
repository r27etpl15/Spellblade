// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAssets/StartUpData/DataAsset_StartUpDataBase.h"
#include "WarriorTypes/WarriorStructTypes.h"

#include "DataAsset_HeroStartUpData.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API UDataAsset_HeroStartUpData : public UDataAsset_StartUpDataBase
{
	GENERATED_BODY()
	
public:
	// 重写父类函数用于赋予英雄需要激活的能力
	virtual void GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InAscToGive, const int32 ApplyLevel = 1) override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category="StartUpData", meta=(TitleProperty = "InputTag"))
	TArray<FWarriorHeroAbilitySet> HeroStartUpAbilitySets;
};
