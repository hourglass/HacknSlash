// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FocusTarget.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_FocusTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// 바라볼 목표를 설정하는 함수
	// 현재 액터가 인터페이스를 가지고 있는지 확인
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// IAIBehavior 인터페이스로 캐스팅
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

		// Behavior Tree의 블랙보드를 가져오는 함수
		// 블랙보드에 등록된 타겟을 가져오기
		auto TargetActor = AIBehaviorObject->GetActorFromBB(Target.SelectedKeyName);
		if (TargetActor)
		{
			// AAIController 캐스팅
			auto AIController = Cast<AAIController>(OwnerComp.GetOwner());
			if (AIController)
			{
				// 컨트롤러를 통해 타겟을 바라보도록 설정
				AIController->SetFocus(TargetActor);

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
