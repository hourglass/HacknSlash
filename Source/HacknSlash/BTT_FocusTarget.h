// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HacknSlash.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FocusTarget.generated.h"


/**
 * 
 */
UCLASS()
class HACKNSLASH_API UBTT_FocusTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	FBlackboardKeySelector Target;
};
