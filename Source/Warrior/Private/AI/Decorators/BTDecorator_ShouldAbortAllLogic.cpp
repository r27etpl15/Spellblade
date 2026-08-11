// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorators/BTDecorator_ShouldAbortAllLogic.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "WarriorBlueprintFunctionLibrary.h"
#include "AIController.h"
#include "WarriorGameplayTags.h"

#include "WarriorDebugHelper.h"
#include "BehaviorTree/BehaviorTree.h"

UBTDecorator_ShouldAbortAllLogic::UBTDecorator_ShouldAbortAllLogic()
{
	NodeName = TEXT("Native Should Abort All Logic");
	
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
	
	InTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InTargetActorKey), 
		AActor::StaticClass());
	InDistToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InDistToTargetKey));
}

FString UBTDecorator_ShouldAbortAllLogic::GetStaticDescription() const
{
	FString KeyDescription = FString::Printf(TEXT("%s: \n\n"), *Super::GetStaticDescription());
	KeyDescription += FString::Printf(TEXT("InTargetActorKey: %s\n"), 
		*InTargetActorKey.SelectedKeyName.ToString());
	KeyDescription += FString::Printf(TEXT("InDistToActorKey: %s"), 
	*InDistToTargetKey.SelectedKeyName.ToString());
	
	return KeyDescription;
}

void UBTDecorator_ShouldAbortAllLogic::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetActorKey.ResolveSelectedKey(*BBAsset);
		InDistToTargetKey.ResolveSelectedKey(*BBAsset);
	}
}

bool UBTDecorator_ShouldAbortAllLogic::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InTargetActorKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(ActorObject);
	
	float DistToTarget = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(InDistToTargetKey.SelectedKeyName);
	
	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();
	
	bool bIsTargetActorDead = false;
	bool bIsOwningAIDead = false;
	
	if (TargetActor && OwningPawn)
	{
		if (UWarriorBlueprintFunctionLibrary::NativeDoesActorHaveTag(TargetActor, 
		WarriorGameplayTags::Shared_Status_Dead))
		{
			bIsTargetActorDead = true;
		}
		if (UWarriorBlueprintFunctionLibrary::NativeDoesActorHaveTag(OwningPawn, 
			WarriorGameplayTags::Shared_Status_Dead))
		{
			bIsOwningAIDead = true;
		}
	
		if (FMath::IsNearlyZero(DistToTarget) || bIsTargetActorDead || bIsOwningAIDead)
		{
			return true;
		}
	}
	
	return false;
}

void UBTDecorator_ShouldAbortAllLogic::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardComponent)
	{
		FBlackboard::FKey DistKeyID = InDistToTargetKey.GetSelectedKeyID();
		
		BlackboardComponent->RegisterObserver(DistKeyID, this, 
			FOnBlackboardChangeNotification::CreateUObject(this, &ThisClass::OnBlackboardKeyValueChange));
		
		FBlackboard::FKey TargetActorKeyID = InTargetActorKey.GetSelectedKeyID();
		
		BlackboardComponent->RegisterObserver(TargetActorKeyID, this, 
			FOnBlackboardChangeNotification::CreateUObject(this, &ThisClass::OnBlackboardKeyValueChange));
	}

	bNotifyTick = true;
}

void UBTDecorator_ShouldAbortAllLogic::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (BlackboardComponent)
	{
		BlackboardComponent->UnregisterObserversFrom(this);
	}

	bNotifyTick = false;
}

void UBTDecorator_ShouldAbortAllLogic::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!CalculateRawConditionValue(OwnerComp, NodeMemory))
	{
		return;
	}

	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (OwningPawn && UWarriorBlueprintFunctionLibrary::NativeDoesActorHaveTag(
		OwningPawn, WarriorGameplayTags::Shared_Status_Dead))
	{
		OwnerComp.StopLogic(TEXT("ShouldAbortAllLogic"));
	}
	else
	{
		OwnerComp.RequestExecution(this);
	}
}

EBlackboardNotificationResult UBTDecorator_ShouldAbortAllLogic::OnBlackboardKeyValueChange(
	const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(Blackboard.GetBrainComponent());
	
	if (BehaviorTreeComponent == nullptr)
	{
		return EBlackboardNotificationResult::RemoveObserver;
	}
	
	if (ChangedKeyID == InDistToTargetKey.GetSelectedKeyID())
	{
		BehaviorTreeComponent->RequestExecution(this);
	}
	
	if (ChangedKeyID == InTargetActorKey.GetSelectedKeyID())
	{
		BehaviorTreeComponent->RequestExecution(this);
	}
	
	return EBlackboardNotificationResult::ContinueObserving;
}
