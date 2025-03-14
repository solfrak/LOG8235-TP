// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTBridge.h"
#include "SDTPathFollowingComponent.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

#include "SDTUtils.h"
#include "EngineUtils.h"

#include "NavigationSystem.h"

ASDTAIController::ASDTAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
           
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    //Move to target depending on current behavior
    APawn* pawn = GetPawn();

    UPathFollowingComponent* path = GetPathFollowingComponent();
    if (path == nullptr)
    {
        return;
    }

    switch (m_PedestrianState)
    {
        case PedestrianState::SPAWNED:
        {
            FString tag("WaitPoint_Bridge_");
            AActor* actor = FindActorWithTag(tag);
            
            if (actor != nullptr)
            {
                // TODO : Agents wants to move towards actor
                MoveToActor(actor);

                m_ReachedTarget = false;
                m_PedestrianState = PedestrianState::GO_TO_BRIDGE;
            }
            break;
        }
        case PedestrianState::GO_TO_BRIDGE:
        {
            // Nothing to do
            break;
        }
        case PedestrianState::WAIT_AT_BRIDGE:
        {
            // Check if bridge is down
            FString tag("Bridge_");
            AActor* actor = FindActorWithTag(tag);
            ASDTBridge* bridge = Cast<ASDTBridge>(actor);
            
            // Once the bridge is down, we go through
            if (bridge != nullptr && bridge->GetState() == EBridgeState::BRIDGE_DOWN)
            {
                m_PedestrianState = PedestrianState::GO_TO_DESPAWN;
            }

            break;
        }
        case PedestrianState::GO_TO_DESPAWN:
        {
            FString tag("Despawn_");
            AActor* actor = FindActorWithTag(tag);
            
            if (actor != nullptr)
            {
                // TODO : Agents wants to move towards actor
                MoveToActor(actor);
                m_ReachedTarget = false;
            }
            
            break;
        }
        case PedestrianState::DESPAWN:
        {
            UnPossess();
            Destroy();

            pawn->Destroy();
            break;
        }
    }
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);

    m_ReachedTarget = true;

    // Safeguard
    if (Result.Code == EPathFollowingResult::Aborted)
    {
        return;
    }

    switch (m_PedestrianState)
    {
        case PedestrianState::GO_TO_BRIDGE:
        {
            m_PedestrianState = PedestrianState::WAIT_AT_BRIDGE;
            break;
        }
        case PedestrianState::GO_TO_DESPAWN:
        {
            m_PedestrianState = PedestrianState::DESPAWN;
            break;
        }
    }
}

void ASDTAIController::ShowNavigationPath()
{
    UPathFollowingComponent* PathComponent = GetPathFollowingComponent();
    if (!PathComponent)
        return;

    FNavPathSharedPtr NavPath = PathComponent->GetPath();
    if (!NavPath.IsValid())
        return;

    const TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();
    UWorld* World = GetWorld();
    if (!World)
        return;

    for (int32 i = 0; i < PathPoints.Num(); i++)
    {
        FVector CurrentLocation = PathPoints[i].Location;

        DrawDebugSphere(World, CurrentLocation, 25.f, 12, FColor::Blue, false, 0.f, 0, 2.f);

        if (i < PathPoints.Num() - 1)
        {
            FVector NextLocation = PathPoints[i + 1].Location;
            DrawDebugLine(World, CurrentLocation, NextLocation, FColor::Yellow, false, 0.f, 0, 2.f);
        }
    }
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();

    m_ReachedTarget = false;
}