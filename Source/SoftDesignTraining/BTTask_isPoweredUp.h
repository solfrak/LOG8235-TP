// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_isPoweredUp.generated.h"

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API UBTTask_isPoweredUp : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	UBTTask_isPoweredUp();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
