// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_SetRecoilLocation.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_SetRecoilLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

		auto TargetActor = AIBehaviorObject->GetActorFromBB(FName("TargetActor"));
		if (TargetActor)
		{
			auto AIController = Cast<AAIController>(OwnerComp.GetOwner());
			if (AIController)
			{
				float ReCoilDistance = FMath::RandRange(250.f, 500.f);
				auto OrinLoc = AIController->GetPawn()->GetActorLocation();
				auto Backward = OrinLoc - TargetActor->GetActorLocation();
				Backward.Normalize();

				AIBehaviorObject->SetDestination(OrinLoc + Backward * ReCoilDistance);

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}

