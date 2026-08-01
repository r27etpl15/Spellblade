// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorators/BTDecorator_DoesActorHasTag.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "WarriorBlueprintFunctionLibrary.h"

#include "WarriorDebugHelper.h"

UBTDecorator_DoesActorHasTag::UBTDecorator_DoesActorHasTag()
{
	NodeName = TEXT("Native Does Actor Has Tag");
	
	INIT_DECORATOR_NODE_NOTIFY_FLAGS();
	
	bInverseConditionCheck = false;
	
	FlowAbortMode = EBTFlowAbortMode::Self;
	
	InActorKeyToCheck.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InActorKeyToCheck), 
		AActor::StaticClass());
}

FString UBTDecorator_DoesActorHasTag::GetStaticDescription() const
{
	FString KeyDescription = FString::Printf(TEXT("%s: \n\n"), *Super::GetStaticDescription());
	KeyDescription += FString::Printf(TEXT("InActorKeyToCheck: %s\n"), 
		*InActorKeyToCheck.SelectedKeyName.ToString());
	KeyDescription += FString::Printf(TEXT("InTagToCheck: %s\n"), *InTagToCheck.ToString());
	KeyDescription += FString::Printf(TEXT("bInverseConditionCheck: %s"), 
		bInverseConditionCheck ? TEXT("True") : TEXT("False"));
	
	return KeyDescription;
}

void UBTDecorator_DoesActorHasTag::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InActorKeyToCheck.ResolveSelectedKey(*BBAsset);
	}
}

bool UBTDecorator_DoesActorHasTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp,
															  uint8* NodeMemory) const
{
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InActorKeyToCheck.SelectedKeyName);
	AActor* ActorToCheck = Cast<AActor>(ActorObject);
	
	if (ActorToCheck)
	{
		const bool bResult = UWarriorBlueprintFunctionLibrary::NativeDoesActorHaveTag(ActorToCheck, InTagToCheck);
		
		return bInverseConditionCheck ? !bResult : bResult;
	}
	
	return false;
}
