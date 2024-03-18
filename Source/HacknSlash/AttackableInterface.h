// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AttackableInterface.generated.h"

/**
 *
 */
UINTERFACE()
class  HACKNSLASH_API UAttackableInterface : public UInterface
{
	GENERATED_BODY()
};

class HACKNSLASH_API IAttackableInterface
{
	GENERATED_BODY()

public:
	virtual void AttackCheck();
	virtual void AttackCheckEnd();
	virtual bool IsFinalAttack();
};