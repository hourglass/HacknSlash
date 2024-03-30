// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_SetRecoilLocation.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_SetRecoilLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// �÷��̾�Լ� �ް����� �ϴ� �Լ�
	// ���� ���Ͱ� �������̽��� ������ �ִ��� Ȯ��
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// IAIBehavior �������̽��� ĳ����
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

		// �����忡 ��ϵ� Ÿ���� ��������
		auto TargetActor = AIBehaviorObject->GetActorFromBB(FName("TargetActor"));
		if (TargetActor)
		{
			// AAIController ĳ����
			auto AIController = Cast<AAIController>(OwnerComp.GetOwner());
			if (AIController)
			{
				// �ް������� �Ÿ��� ���� ���
				float ReCoilDistance = FMath::RandRange(250.f, 500.f);

				// ���� ��ġ
				auto OrinLoc = AIController->GetPawn()->GetActorLocation();

				// �̵��� ���� (�ڽ� ��ġ ���� - Ÿ���� ��ġ ����)
				auto Backward = OrinLoc - TargetActor->GetActorLocation();
				Backward.Normalize();

				// ���� ��ġ���� ���� ��µ� �Ÿ���ŭ �̵�
				AIBehaviorObject->SetDestination(OrinLoc + (Backward * ReCoilDistance));

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}

