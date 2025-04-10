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
    TRACE_CPUPROFILER_EVENT_SCOPE(UBTService_TryDetectPlayer::TickNode);
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
        //get les vieux states
        bool prev_state_los = BB->GetValueAsBool(FName("CanSeePlayer"));
        bool prev_state_pwd = BB->GetValueAsBool(FName("IsPoweredUp"));
        //udate
        BB->SetValueAsBool(FName("CanSeePlayer"), bCanSeePlayer);
        BB->SetValueAsVector(FName("TargetPosition"), MyAICon->m_TargetPosition);
        BB->SetValueAsBool(FName("IsPoweredUp"), bIsPoweredUp);
        //if state updated, alors change state avec aistateinterrupted et mettre reachedtarget a true

        //si on le voit pas et quon le voit -> OK
        //si on le voit et quon le voit plus -> OK
        //si on le voit pas et quon le voit et quil etait pas powered up et quil est devenu powered up -> OK
        //si on le voit pas et quon le voit et quil etait powered up et quil est pas powered up -> OK
        //si on le voit et quon le voit plus et quil etait pas powered up et quil est devenu powered up -> DONT NEED
        //si on le voit et quon le voit plus et quil etait powered up et quil est pas powered up -> DONT NEED

        if (prev_state_los != bCanSeePlayer ||  (bCanSeePlayer && prev_state_pwd != bIsPoweredUp))
        {
            //XDD!!!!!
            if (!MyAICon->InAir) {
                MyAICon->AIStateInterrupted();
                BB->SetValueAsBool(FName("ReachedTarget"), true);
            }
            else {
                BB->SetValueAsBool(FName("ReachedTarget"), MyAICon->m_ReachedTarget);
            }
        }
    }
}