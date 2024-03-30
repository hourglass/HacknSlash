// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_AttackTarget.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_AttackTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// AI 공격 처리 함수
	// 현재 액터가 인터페이스를 가지고 있는지 확인
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// IAIBehavior 인터페이스로 캐스팅
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);
		AIBehaviorObject->AttackTarget();

		// 콤보 시작 시 공격 방향을 유지하도록 포커스 해제
		AIBehaviorObject->OnComboStart.AddLambda([&]()
			{
				ClearFocus(OwnerComp.GetAIOwner());
			});

		// 콤보 종료 후 다시 공격 대상을 바라보도록 포커스 설정
		AIBehaviorObject->OnComboEnded.AddLambda([&]()
			{
				SetFocus(OwnerComp.GetAIOwner(), OwnerComp.GetAIOwner()->GetPawn());
			});

		// 공격 도중에 현재 Task를 벗어나지 않도록
		// 공격이 끝난 후에 Succeeded 반환하는 이벤트 수행
		AIBehaviorObject->OnAttackEnded.AddLambda([&]()
			{
				ClearFocus(OwnerComp.GetAIOwner());
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			});
		return  EBTNodeResult::InProgress;
	}

	return  EBTNodeResult::Failed;
}

// 공격 대상 포커스 해제 함수
void UBTT_AttackTarget::ClearFocus(AAIController* AIController)
{
	if (IsValid(AIController))
	{
		AIController->ClearFocus(EAIFocusPriority::LastFocusPriority);
	}
}

// 공격 대상 포커스 설정 함수
void UBTT_AttackTarget::SetFocus(AAIController* AIController, APawn* MyPawn)
{
	if (IsValid(AIController) && IsValid(MyPawn))
	{
		// 현재 액터가 인터페이스를 가지고 있는지 확인
		bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
		if (HasAIBehavior)
		{
			// IAIBehavior 인터페이스로 캐스팅
			auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

			// AAIController 캐스팅
			auto TagetActor = AIBehaviorObject->GetActorFromBB(FName("TargetActor"));
			if (TagetActor)
			{
				// 컨트롤러를 통해 타겟을 바라보도록 설정
				AIController->SetFocus(TagetActor);
			}
		}
	}
}
