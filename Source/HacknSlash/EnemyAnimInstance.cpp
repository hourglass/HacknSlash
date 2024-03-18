// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"

UEnemyAnimInstance::UEnemyAnimInstance()
{
	ConstructorHelpers::FObjectFinder<UAnimMontage> AM_BattleCry(TEXT("/Game/Animations/Montage/AM_BattleCry.AM_BattleCry"));
	if (AM_BattleCry.Succeeded())
	{
		BattleCryMontage = AM_BattleCry.Object;
	}
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto MyPawn = TryGetPawnOwner();
	if (IsValid(MyPawn))
	{
		MoveSpeed = MyPawn->GetVelocity().Size();

		// Calc MoveBackward
		auto Forward = MyPawn->GetActorForwardVector();
		auto MoveDirection = MyPawn->GetVelocity();

		auto Direction = FVector::DotProduct(Forward, MoveDirection);
		(Direction < 0) ? bMoveBackward = true : bMoveBackward = false;

		if (bPrevMoveDirection != bMoveBackward)
		{
			OnChangeMoveDirection.Broadcast();
		}

		bPrevMoveDirection = bMoveBackward;
	}
}

void UEnemyAnimInstance::PlayBattleCryMontage()
{
	if (IsValid(BattleCryMontage))
	{
		if (!Montage_IsPlaying(BattleCryMontage))
		{
			Montage_Play(BattleCryMontage);
		}
	}
}

bool UEnemyAnimInstance::GetMoveBackward()
{
	return bMoveBackward;
}