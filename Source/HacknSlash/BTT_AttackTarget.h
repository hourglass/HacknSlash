// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_AttackTarget.generated.h"

/**
 * 
 */
UCLASS()
class HACKNSLASH_API UBTT_AttackTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	void ClearFocus(class AAIController* AIController);
	void SetFocus(class AAIController* AIController, APawn* MyPawn);
};
