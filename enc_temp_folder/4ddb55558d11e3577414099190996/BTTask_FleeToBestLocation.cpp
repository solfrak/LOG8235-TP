// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_FleeToBestLocation.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"

EBTNodeResult::Type UBTTask_FleeToBestLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        aiController->LeavePursuitGroup();
		aiController->MoveToBestFleeLocation();
		return EBTNodeResult::Succeeded;

    }
    return EBTNodeResult::Failed;
}