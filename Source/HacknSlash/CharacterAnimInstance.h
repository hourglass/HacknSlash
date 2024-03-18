// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterAnimInstance.generated.h"

DECLARE_MULTICAST_DELEGATE(FNotifyReceiveDelegate);
/**
 * 
 */
UCLASS()
class HACKNSLASH_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UCharacterAnimInstance();

	void PlayHitMontage();
	void PlayAttackMontage(float AttackSpeed);
	void JumpToAttackMontageSection(int32 NewSection);
	void SetAttackMontage(UAnimMontage* NewAttackMontage);
	void SetIsDead(bool State);

	UFUNCTION()
	void AnimNotify_SendSwing();

	UFUNCTION()
	void AnimNotify_SendSmash();

	UFUNCTION()
	void AnimNotify_ComboCheck();

	UFUNCTION()
	void AnimNotify_AttackStart();

	FNotifyReceiveDelegate OnSendSwing;
	FNotifyReceiveDelegate OnSendSmash;
	FNotifyReceiveDelegate OnComboCheck;
	FNotifyReceiveDelegate OnAttackStart;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool bIsDead;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	UAnimMontage* HitMontage;

	UPROPERTY()
	UAnimMontage* AttackMontage;
};
