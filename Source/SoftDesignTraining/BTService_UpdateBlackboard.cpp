// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_UpdateBlackboard.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "SDTUtils.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateBlackboard::UBTService_UpdateBlackboard()
{
    NodeName = "UpdateBlackboard";
    bCreateNodeInstance = true;
}


void UBTService_UpdateBlackboard::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_UpdateBlackboard::TickNode);
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    ASDTAIController* MyAICon = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!MyAICon) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    // Lecture passive des états du AIController
    const bool bCanSeePlayer = MyAICon->HasLineOfSightToPlayer();
    const bool bIsPoweredUp = SDTUtils::IsPlayerPoweredUp(MyAICon->GetWorld());
    const bool bIsInGroup = MyAICon->IsInPursuitGroup();
    const bool bReachedTarget = MyAICon->m_ReachedTarget;
	FVector targetPos = FVector::Zero();
    if (MyAICon->current_interest_point)
    {
		targetPos = MyAICon->current_interest_point->GetActorLocation();
    }

    // Mise à jour du Blackboard
    BB->SetValueAsBool(FName("CanSeePlayer"), bCanSeePlayer);
    BB->SetValueAsBool(FName("IsPoweredUp"), bIsPoweredUp);
    BB->SetValueAsBool(FName("IsInGroup"), bIsInGroup);
}
