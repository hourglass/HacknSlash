// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitEventInterface.generated.h"

/**
 * 
 */
UINTERFACE()
class HACKNSLASH_API UHitEventInterface : public UInterface
{
public:
	GENERATED_BODY()
};

class HACKNSLASH_API IHitEventInterface
{
	GENERATED_BODY()

public:
	virtual void HitFinalAttack();
};
