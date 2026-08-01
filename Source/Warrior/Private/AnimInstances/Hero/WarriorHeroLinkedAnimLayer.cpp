// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/Hero/WarriorHeroLinkedAnimLayer.h"
#include "AnimInstances/Hero/WarriorHeroAnimInstance.h"

UWarriorHeroAnimInstance* UWarriorHeroLinkedAnimLayer::GetWarriorHeroAnimInstance() const
{
	 return Cast<UWarriorHeroAnimInstance>(GetOwningComponent()->GetAnimInstance());
}
