// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_CollectPickup.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"

EBTNodeResult::Type UBTTask_CollectPickup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        aiController->MoveToRandomCollectible();
        return EBTNodeResult::Succeeded;

    }
    return EBTNodeResult::Failed;
}

