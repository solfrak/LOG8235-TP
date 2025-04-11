// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_isPoweredUp.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingCharacter.h"
#include "SDTUtils.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTTask_isPoweredUp::UBTTask_isPoweredUp()
{
    NodeName = "IsPoweredUp";
}

EBTNodeResult::Type UBTTask_isPoweredUp::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (SDTUtils::IsPlayerPoweredUp(GetWorld())) {
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}