// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterAnimInstance.h"
#include "HnsCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class HACKNSLASH_API UHnsCharacterAnimInstance : public UCharacterAnimInstance
{
	GENERATED_BODY()

public:
	UHnsCharacterAnimInstance();
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void PlayRollMontage();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	UAnimMontage* RollMontage;
};
