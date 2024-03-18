// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAnimInstance.h"

UCharacterAnimInstance::UCharacterAnimInstance()
{
	// Variable Setting //
	MoveSpeed = 0.f;
	bIsDead = false;

	ConstructorHelpers::FObjectFinder<UAnimMontage> AM_Hit(TEXT("/Game/Animations/Montage/AM_Hit.AM_Hit"));
	if (AM_Hit.Succeeded())
	{
		HitMontage = AM_Hit.Object;
	}
}

void UCharacterAnimInstance::PlayHitMontage()
{
	Montage_Play(HitMontage, 1.0f);
}

void UCharacterAnimInstance::PlayAttackMontage(float AttackSpeed)
{
	if (IsValid(AttackMontage))
	{
		if (!Montage_IsPlaying(AttackMontage))
		{
			Montage_Play(AttackMontage, AttackSpeed);
		}
	}
}

void UCharacterAnimInstance::JumpToAttackMontageSection(int32 NewSection)
{
	if (IsValid(AttackMontage))
	{
		FName SectionName = (*(FString(TEXT("Attack")) + FString::FromInt(NewSection)));

		Montage_JumpToSection(SectionName, AttackMontage);
	}
}

void UCharacterAnimInstance::SetAttackMontage(UAnimMontage* NewAttackMontage)
{
	AttackMontage = NewAttackMontage;
}

void UCharacterAnimInstance::SetIsDead(bool State)
{
	bIsDead = State;
}


void UCharacterAnimInstance::AnimNotify_SendSwing()
{
	OnSendSwing.Broadcast();
}

void UCharacterAnimInstance::AnimNotify_SendSmash()
{
	OnSendSmash.Broadcast();
}

void UCharacterAnimInstance::AnimNotify_ComboCheck()
{
	OnComboCheck.Broadcast();
}

void UCharacterAnimInstance::AnimNotify_AttackStart()
{
	OnAttackStart.Broadcast();
}