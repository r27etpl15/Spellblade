// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/WarriorProjectileBase.h"
#include "SpellbladeMagicMissile.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API ASpellbladeMagicMissile : public AWarriorProjectileBase
{
	GENERATED_BODY()
	
public:
	ASpellbladeMagicMissile();
	
	virtual void Tick(float DeltaSecond) override;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName = "On Spawn Projectile Overlap FX"))
	void BP_OnSpawnProjectileOverlapFX(const FVector& InOverlapLocation);
	
	void GetAvailableActorsToFly();
	AActor* GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors);
	
	UPROPERTY()
	TArray<AActor*> AvailableActorsToFly;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentFaceActor;
	
	UPROPERTY(EditDefaultsOnly, Category="Missile")
	int32 MaxOverlapNums = 6;
	
	UPROPERTY(BlueprintReadOnly, Category="Missile")
	int32 CurrentOverlapNums = 0;
	
	UPROPERTY(EditDefaultsOnly, Category="Missile")
	float BoxTraceDistance = 5000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Missile")
	FVector TraceBoxSize = FVector(5000.0f, 5000.0f, 300.0f);
	
	UPROPERTY(EditDefaultsOnly, Category="Missile")
	TArray<TEnumAsByte<EObjectTypeQuery>> BoxTraceChannel;
	
	UPROPERTY(EditDefaultsOnly, Category="Missile")
	bool bShowPersistentDebugShape = false;
	
	UPROPERTY()
	bool bShouldFaceToTarget = false;
	
	UPROPERTY(EditDefaultsOnly)
	float RotationInterpSpeed = 10.0f;
	
	UPROPERTY()
	TArray<AActor*> HitTargets;
};
