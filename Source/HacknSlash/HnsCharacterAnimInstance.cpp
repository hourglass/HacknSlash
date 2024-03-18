// Fill out your copyright notice in the Description page of Project Settings.

#include "HnsCharacterAnimInstance.h"

UHnsCharacterAnimInstance::UHnsCharacterAnimInstance()
{
	ConstructorHelpers::FObjectFinder<UAnimMontage> AM_Roll(TEXT("/Game/Animations/Montage/AM_Roll.AM_Roll"));
	if (AM_Roll.Succeeded())
	{
		RollMontage = AM_Roll.Object;
	}
}

void UHnsCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto MyPawn = TryGetPawnOwner();
	if (IsValid(MyPawn))
	{
		MoveSpeed = MyPawn->GetVelocity().Size();
	}
}

void UHnsCharacterAnimInstance::PlayRollMontage()
{
	if (!Montage_IsPlaying(RollMontage))
	{
		Montage_Play(RollMontage, 1.2f);
	}
}