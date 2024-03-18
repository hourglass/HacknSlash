// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AIBehaviorInterface.generated.h"

DECLARE_MULTICAST_DELEGATE(FAttackEvent);
/**
 * 
 */
UINTERFACE()
class  HACKNSLASH_API UAIBehaviorInterface : public UInterface
{
	GENERATED_BODY()
};

class HACKNSLASH_API IAIBehaviorInterface
{
	GENERATED_BODY()

public:
	virtual AActor* GetActorFromBB(FName KeyName);
	virtual void SetDestination(FVector Destination);

	virtual void FindWeapon();
	virtual void EquipWeapon();
	virtual void AttackTarget();

	FAttackEvent OnComboStart;
	FAttackEvent OnComboEnded;
	FAttackEvent OnAttackEnded;
};