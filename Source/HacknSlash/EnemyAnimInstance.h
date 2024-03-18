// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HnsCharacterAnimInstance.h"
#include "EnemyAnimInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FAnimInstanceDelegate);
/**
 * 
 */
UCLASS()
class HACKNSLASH_API UEnemyAnimInstance : public UCharacterAnimInstance
{
	GENERATED_BODY()
	
public:
	UEnemyAnimInstance();

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void PlayBattleCryMontage();
	bool GetMoveBackward();

	FAnimInstanceDelegate OnChangeMoveDirection;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool bMoveBackward;

	UPROPERTY()
	bool bPrevMoveDirection;

	UPROPERTY()
	UAnimMontage* BattleCryMontage;
};
