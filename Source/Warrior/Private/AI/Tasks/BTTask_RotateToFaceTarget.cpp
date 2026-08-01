// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_RotateToFaceTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_RotateToFaceTarget::UBTTask_RotateToFaceTarget()
{
	NodeName = TEXT("Native Rotate To Face Target Actor");
	
	INIT_TASK_NODE_NOTIFY_FLAGS();
	
	AnglePrecision = 10.0f;
	RotationInterpSpeed = 5.0f;
	
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;
	
	InTargetToFaceKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InTargetToFaceKey), 
		AActor::StaticClass());
}

void UBTTask_RotateToFaceTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetToFaceKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTTask_RotateToFaceTarget::GetInstanceMemorySize() const
{
	return sizeof(FRotateToFaceTargetTaskMemory);
}

FString UBTTask_RotateToFaceTarget::GetStaticDescription() const
{
	FString KeyDescription = FString::Printf(TEXT("Native Rotate To Face Target Actor:\n\n"));
	KeyDescription += FString::Printf(TEXT("InTargetToFaceKey: %s\n"), 
		*InTargetToFaceKey.SelectedKeyName.ToString());
	KeyDescription += FString::Printf(TEXT("AnglePrecision: %f\n"), AnglePrecision);
	KeyDescription += FString::Printf(TEXT("RotationInterpSpeed: %f"), RotationInterpSpeed);
	
	return KeyDescription;
}

EBTNodeResult::Type UBTTask_RotateToFaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InTargetToFaceKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(ActorObject);
	
	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();
	
	FRotateToFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);
	ensure(Memory);
	
	Memory->OwningPawn = OwningPawn;
	Memory->TargetActor = TargetActor;
	
	if (!Memory->IsValid())
	{
		return EBTNodeResult::Failed;
	}
	
	if (HasReachedAnglePrecision(OwningPawn, TargetActor))
	{
		Memory->Reset();
		
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::InProgress;
}

void UBTTask_RotateToFaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	FRotateToFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);
	
	if (!Memory->IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
	
	if (HasReachedAnglePrecision(Memory->OwningPawn.Get(), Memory->TargetActor.Get()))
	{
		Memory->Reset();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else
	{
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
			Memory->OwningPawn->GetActorLocation(),Memory->TargetActor->GetActorLocation());
		const FRotator TargetRotation = FMath::RInterpTo(Memory->OwningPawn->GetActorRotation(), 
			LookAtRotation, DeltaSeconds, RotationInterpSpeed);
		
		Memory->OwningPawn->SetActorRotation(TargetRotation);
	}
}

bool UBTTask_RotateToFaceTarget::HasReachedAnglePrecision(APawn* QueryPawn, AActor* TargetActor) const
{
	const FVector OwnerForward = QueryPawn->GetActorForwardVector();
	const FVector OwnerToTargetNormalized = (TargetActor->GetActorLocation() - 
		QueryPawn->GetActorLocation()).GetSafeNormal();
	
	const float DotResult = FVector::DotProduct(OwnerForward, OwnerToTargetNormalized);
	const float AngleDiff = UKismetMathLibrary::DegAcos(DotResult);
	
	return AngleDiff <= AnglePrecision;
}
