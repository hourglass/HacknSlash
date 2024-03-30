// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_AttackTarget.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_AttackTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// AI ���� ó�� �Լ�
	// ���� ���Ͱ� �������̽��� ������ �ִ��� Ȯ��
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// IAIBehavior �������̽��� ĳ����
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);
		AIBehaviorObject->AttackTarget();

		// �޺� ���� �� ���� ������ �����ϵ��� ��Ŀ�� ����
		AIBehaviorObject->OnComboStart.AddLambda([&]()
			{
				ClearFocus(OwnerComp.GetAIOwner());
			});

		// �޺� ���� �� �ٽ� ���� ����� �ٶ󺸵��� ��Ŀ�� ����
		AIBehaviorObject->OnComboEnded.AddLambda([&]()
			{
				SetFocus(OwnerComp.GetAIOwner(), OwnerComp.GetAIOwner()->GetPawn());
			});

		// ���� ���߿� ���� Task�� ����� �ʵ���
		// ������ ���� �Ŀ� Succeeded ��ȯ�ϴ� �̺�Ʈ ����
		AIBehaviorObject->OnAttackEnded.AddLambda([&]()
			{
				ClearFocus(OwnerComp.GetAIOwner());
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			});
		return  EBTNodeResult::InProgress;
	}

	return  EBTNodeResult::Failed;
}

// ���� ��� ��Ŀ�� ���� �Լ�
void UBTT_AttackTarget::ClearFocus(AAIController* AIController)
{
	if (IsValid(AIController))
	{
		AIController->ClearFocus(EAIFocusPriority::LastFocusPriority);
	}
}

// ���� ��� ��Ŀ�� ���� �Լ�
void UBTT_AttackTarget::SetFocus(AAIController* AIController, APawn* MyPawn)
{
	if (IsValid(AIController) && IsValid(MyPawn))
	{
		// ���� ���Ͱ� �������̽��� ������ �ִ��� Ȯ��
		bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
		if (HasAIBehavior)
		{
			// IAIBehavior �������̽��� ĳ����
			auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

			// AAIController ĳ����
			auto TagetActor = AIBehaviorObject->GetActorFromBB(FName("TargetActor"));
			if (TagetActor)
			{
				// ��Ʈ�ѷ��� ���� Ÿ���� �ٶ󺸵��� ����
				AIController->SetFocus(TagetActor);
			}
		}
	}
}
