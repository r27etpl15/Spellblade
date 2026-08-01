// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_DoesActorHasTag.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API UBTDecorator_DoesActorHasTag : public UBTDecorator
{
	GENERATED_BODY()
	
private:
	UBTDecorator_DoesActorHasTag();
	
protected:
	//~ Begin UBTNode Interface.
	virtual FString GetStaticDescription() const override;
	//~ End UBTNode Interface.
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	UPROPERTY(EditAnywhere, Category="Default")
	FBlackboardKeySelector InActorKeyToCheck;
	
	UPROPERTY(EditAnywhere, Category="Default")
	FGameplayTag InTagToCheck;
	
	UPROPERTY(EditAnywhere, Category="Default")
	bool bInverseConditionCheck;
};
