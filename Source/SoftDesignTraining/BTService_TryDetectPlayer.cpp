// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_TryDetectPlayer.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "SDTUtils.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_TryDetectPlayer::UBTService_TryDetectPlayer()
{
    bCreateNodeInstance = true;
}


void UBTService_TryDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AICon = OwnerComp.GetAIOwner();
    ASDTAIController* MyAICon = Cast<ASDTAIController>(AICon);

    if (!MyAICon) return;

    bool bCanSeePlayer = MyAICon->HasLineOfSightToPlayer();

    bool bIsPoweredUp = SDTUtils::IsPlayerPoweredUp(MyAICon->GetWorld());

    // Update du Blackboard
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsBool(FName("CanSeePlayer"), bCanSeePlayer);
        BB->SetValueAsBool(FName("IsPoweredUp"), bIsPoweredUp);
    }
}