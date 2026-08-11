// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_ShouldAbortAllLogic.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API UBTDecorator_ShouldAbortAllLogic : public UBTDecorator
{
	GENERATED_BODY()
	
private:
	UBTDecorator_ShouldAbortAllLogic();
	
protected:
	//~ Begin UBTNode Interface.
	virtual FString GetStaticDescription() const override;
	//~ End UBTNode Interface.
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, 
		FBlackboard::FKey ChangedKeyID);
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector InTargetActorKey;
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector InDistToTargetKey;
};
