// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_IsReachTarget.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"

EBTNodeResult::Type UBTTask_IsReachTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner()))
    {
        if (aiController->m_ReachedTarget)
            return EBTNodeResult::Succeeded;

        return EBTNodeResult::Failed;
    }
    return EBTNodeResult::Failed;

}
