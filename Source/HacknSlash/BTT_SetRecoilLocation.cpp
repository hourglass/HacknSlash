// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_SetRecoilLocation.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_SetRecoilLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// 플레이어에게서 뒷걸음질 하는 함수
	// 현재 액터가 인터페이스를 가지고 있는지 확인
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// IAIBehavior 인터페이스로 캐스팅
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

		// 블랙보드에 등록된 타겟을 가져오기
		auto TargetActor = AIBehaviorObject->GetActorFromBB(FName("TargetActor"));
		if (TargetActor)
		{
			// AAIController 캐스팅
			auto AIController = Cast<AAIController>(OwnerComp.GetOwner());
			if (AIController)
			{
				// 뒷걸음질할 거리를 랜덤 출력
				float ReCoilDistance = FMath::RandRange(250.f, 500.f);

				// 현재 위치
				auto OrinLoc = AIController->GetPawn()->GetActorLocation();

				// 이동할 방향 (자신 위치 벡터 - 타겟의 위치 벡터)
				auto Backward = OrinLoc - TargetActor->GetActorLocation();
				Backward.Normalize();

				// 현재 위치에서 랜덤 출력된 거리만큼 이동
				AIBehaviorObject->SetDestination(OrinLoc + (Backward * ReCoilDistance));

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}

