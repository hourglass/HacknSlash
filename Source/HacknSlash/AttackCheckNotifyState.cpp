// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackCheckNotifyState.h"
#include "AttackableInterface.h"

void UAttackCheckNotifyState::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	auto Owner = MeshComp->GetOwner();
	if (IsValid(Owner))
	{
		bool HasAttackable = Owner->GetClass()->ImplementsInterface(UAttackableInterface::StaticClass());
		if (HasAttackable)
		{
			auto AttackableObject = Cast<IAttackableInterface>(Owner);
			AttackableObject->AttackCheck();
		}
	}
}

void UAttackCheckNotifyState::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	auto Owner = MeshComp->GetOwner();
	if (IsValid(Owner))
	{
		bool HasAttackable = Owner->GetClass()->ImplementsInterface(UAttackableInterface::StaticClass());
		if (HasAttackable)
		{
			auto AttackableObject = Cast<IAttackableInterface>(Owner);
			AttackableObject->AttackCheckEnd();
		}
	}
}
