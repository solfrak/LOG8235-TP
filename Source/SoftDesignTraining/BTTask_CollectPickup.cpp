// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_CollectPickup.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "Navigation/PathFollowingComponent.h"


EBTNodeResult::Type UBTTask_CollectPickup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!aiController)
        return EBTNodeResult::Failed;

    if (aiController->GetMoveStatus() != EPathFollowingStatus::Idle)
        return EBTNodeResult::Failed;

    aiController->LeavePursuitGroup();

    aiController->MoveToRandomCollectible();
    return EBTNodeResult::Succeeded;
}


