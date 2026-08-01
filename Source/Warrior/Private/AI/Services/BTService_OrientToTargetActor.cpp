// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTService_OrientToTargetActor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"

UBTService_OrientToTargetActor::UBTService_OrientToTargetActor()
{
	NodeName = TEXT("Native Orient Rotation To Target Actor");
	
	INIT_SERVICE_NODE_NOTIFY_FLAGS();
	
	RotationInterpSpeed = 5.0f;
	Interval = 0.0f;
	RandomDeviation = 0.0f;
	
	InTargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InTargetActorKey), 
		AActor::StaticClass());
}

void UBTService_OrientToTargetActor::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTService_OrientToTargetActor::GetStaticDescription() const
{
	FString KeyDescription = FString::Printf(TEXT("Native Orient Rotation To Target Actor %s\n\n"), 
		*Super::GetStaticDescription());
	KeyDescription += FString::Printf(TEXT("InTargetActorKey: %s\n"), *InTargetActorKey.SelectedKeyName.ToString());
	KeyDescription += FString::Printf(TEXT("RotationInterpSpeed: %f"), RotationInterpSpeed);
	
	return KeyDescription;
}

void UBTService_OrientToTargetActor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InTargetActorKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(ActorObject);
	
	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();
	
	if (OwningPawn && TargetActor)
	{
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(OwningPawn->GetActorLocation(), 
			TargetActor->GetActorLocation());
		const FRotator TargetRotation = FMath::RInterpTo(OwningPawn->GetActorRotation(), LookAtRotation, 
			DeltaSeconds, RotationInterpSpeed);
		
		OwningPawn->SetActorRotation(TargetRotation);
	}
}
