// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FocusTarget.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_FocusTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

		auto TargetActor = AIBehaviorObject->GetActorFromBB(Target.SelectedKeyName);
		if (TargetActor)
		{
			auto AIController = Cast<AAIController>(OwnerComp.GetOwner());
			if (AIController)
			{
				AIController->SetFocus(TargetActor);

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
