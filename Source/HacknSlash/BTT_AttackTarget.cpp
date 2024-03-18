// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_AttackTarget.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_AttackTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);
		AIBehaviorObject->AttackTarget();

		AIBehaviorObject->OnComboStart.AddLambda([&]()
			{
				ClearFocus(OwnerComp.GetAIOwner());
			});

		AIBehaviorObject->OnComboEnded.AddLambda([&]()
			{
				SetFocus(OwnerComp.GetAIOwner(), OwnerComp.GetAIOwner()->GetPawn());
			});

		AIBehaviorObject->OnAttackEnded.AddLambda([&]()
			{
				ClearFocus(OwnerComp.GetAIOwner());
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			});
		return  EBTNodeResult::InProgress;
	}

	return  EBTNodeResult::Failed;
}

void UBTT_AttackTarget::ClearFocus(AAIController* AIController)
{
	if (IsValid(AIController))
	{
		AIController->ClearFocus(EAIFocusPriority::LastFocusPriority);
	}
}

void UBTT_AttackTarget::SetFocus(AAIController* AIController, APawn* MyPawn)
{
	if (IsValid(AIController) && IsValid(MyPawn))
	{
		bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
		if (HasAIBehavior)
		{
			auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

			auto TagetActor = AIBehaviorObject->GetActorFromBB(FName("TargetActor"));
			if (TagetActor)
			{
				AIController->SetFocus(TagetActor);
			}
		}
	}
}
