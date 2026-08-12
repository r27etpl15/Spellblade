// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/SpellbladeMagicMissile.h"

#include "WarriorBlueprintFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WarriorDebugHelper.h"
#include "WarriorGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ASpellbladeMagicMissile::ASpellbladeMagicMissile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ProjectileMovementComponent->InitialSpeed = 800.0f;
	ProjectileMovementComponent->MaxSpeed = 1000.0f;
}

void ASpellbladeMagicMissile::Tick(float DeltaSecond)
{
	Super::Tick(DeltaSecond);
	
	if (CurrentFaceActor.IsValid() && bShouldFaceToTarget)
	{
		const FRotator LookAtLocation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), 
			CurrentFaceActor->GetActorLocation());
		const FRotator TargetRotation = FMath::RInterpTo(GetActorRotation(), LookAtLocation,
			DeltaSecond, RotationInterpSpeed);
		
		SetActorRotation(TargetRotation);
		ProjectileMovementComponent->Velocity = GetActorForwardVector() * ProjectileMovementComponent->MaxSpeed;
	}
	else if (!CurrentFaceActor.IsValid() && bShouldFaceToTarget)
	{
		GetAvailableActorsToFly();
	
		CurrentFaceActor = GetNearestTargetFromAvailableActors(AvailableActorsToFly);
	}
}

void ASpellbladeMagicMissile::BeginPlay()
{
	Super::BeginPlay();
	
	GetAvailableActorsToFly();
	
	CurrentFaceActor = GetNearestTargetFromAvailableActors(AvailableActorsToFly);
	
	bShouldFaceToTarget = true;
}

void ASpellbladeMagicMissile::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnProjectileBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep,
	                                SweepResult);
	
	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (UWarriorBlueprintFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
		{
			BP_OnSpawnProjectileOverlapFX(SweepResult.ImpactPoint);
		}
	}
	
	HitTargets.AddUnique(OtherActor);
	GetAvailableActorsToFly();
	
	CurrentFaceActor = GetNearestTargetFromAvailableActors(AvailableActorsToFly);
	if (CurrentFaceActor.IsValid() && CurrentOverlapNums < MaxOverlapNums)
	{
		CurrentOverlapNums++;
		
		bShouldFaceToTarget = true;
		
		SetLifeSpan(5.0f);
	}
	else
	{
		bShouldFaceToTarget = false;
	}
}

void ASpellbladeMagicMissile::GetAvailableActorsToFly()
{
	AvailableActorsToFly.Empty();
	
	TArray<FHitResult> BoxTraceHitResults;
	
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		this,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * BoxTraceDistance,
		TraceBoxSize / 2.0f,
		GetActorForwardVector().ToOrientationRotator(),
		BoxTraceChannel,
		false,
		TArray<AActor*>(),
		bShowPersistentDebugShape ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,
		BoxTraceHitResults,
		true
	);
	
	for (const FHitResult HitResult : BoxTraceHitResults)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (UWarriorBlueprintFunctionLibrary::IsTargetPawnHostile(GetInstigator(), Cast<APawn>(HitActor)))
			{
				if (HitTargets.Contains(HitActor) || UWarriorBlueprintFunctionLibrary::NativeDoesActorHaveTag(HitActor, WarriorGameplayTags::Shared_Status_Dead))
				{
					continue;
				}
				
				AvailableActorsToFly.AddUnique(HitActor);
				
				// Debug::Print(HitActor->GetActorNameOrLabel());
			}
		}
	}
}

AActor* ASpellbladeMagicMissile::GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors)
{
	float NearestDistance = 0.0f;
	
	return UGameplayStatics::FindNearestActor(GetActorLocation(), InAvailableActors, NearestDistance);
}
