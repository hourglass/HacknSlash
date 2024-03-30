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
		// 현재 액터가 인터페이스를 가지고 있는지 확인
		bool HasAttackable = Owner->GetClass()->ImplementsInterface(UAttackableInterface::StaticClass());
		if (HasAttackable)
		{
			// 인터페이스에서 AttackCheck() 함수 실행
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
		// 현재 액터가 인터페이스를 가지고 있는지 확인
		bool HasAttackable = Owner->GetClass()->ImplementsInterface(UAttackableInterface::StaticClass());
		if (HasAttackable)
		{
			// 인터페이스에서 AttackCheckEnd() 함수 실행
			auto AttackableObject = Cast<IAttackableInterface>(Owner);
			AttackableObject->AttackCheckEnd();
		}
	}
}
