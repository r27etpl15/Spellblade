// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTService_GetDistToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_GetDistToTarget::UBTService_GetDistToTarget()
{
	NodeName = TEXT("Native Get Dist To Target");
	
	Interval = 0.2f;
	RandomDeviation = 0.1f;
	
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
	
	InTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InTargetActorKey), 
		AActor::StaticClass());
	OutDistToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, OutDistToTargetKey));
}

void UBTService_GetDistToTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetActorKey.ResolveSelectedKey(*BBAsset);
		OutDistToTargetKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTService_GetDistToTarget::GetStaticDescription() const
{
	FString KeyDescription = FString::Printf(TEXT("Native Get Dist To Target: %s\n\n"), 
		*Super::GetStaticDescription());
	KeyDescription += FString::Printf(TEXT("InTargetActorKey: %s\n"), *InTargetActorKey.SelectedKeyName.ToString());
	KeyDescription += FString::Printf(TEXT("OutDistToTargetKey: %s"), *OutDistToTargetKey.SelectedKeyName.ToString());
	
	return KeyDescription;
}

void UBTService_GetDistToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InTargetActorKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(ActorObject);
	float DistToTarget = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(OutDistToTargetKey.SelectedKeyName);
	
	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();
	
	if (OwningPawn && TargetActor)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsFloat(OutDistToTargetKey.SelectedKeyName, 
			OwningPawn->GetDistanceTo(TargetActor));
	}
}
