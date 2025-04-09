// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_PursuitPlayer.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"

EBTNodeResult::Type UBTTask_PursuitPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        aiController->JoinPursuitGroup();

        if (aiController->m_ReachedTarget)
        {
			aiController->MoveToPlayer();
			return EBTNodeResult::Succeeded;
        }
        else {
			return EBTNodeResult::InProgress;

        }

    }
    return EBTNodeResult::Failed;
}