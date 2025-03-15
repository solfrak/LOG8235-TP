// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SoftDesignTrainingPlayerController.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"

#include "DrawDebugHelpers.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "SDTBridge.h"
#include "SDTBoatOperator.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Navigation/PathFollowingComponent.h"

ASoftDesignTrainingPlayerController::ASoftDesignTrainingPlayerController()
{
    // Make a path following component
    m_PathFollowingComponent = CreateDefaultSubobject<USDTPathFollowingComponent>(TEXT("PathFollowingComponent"));

}

void ASoftDesignTrainingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Move camera
    InputComponent->BindAxis("MoveForward", this, &ASoftDesignTrainingPlayerController::MoveCameraForward);
    InputComponent->BindAxis("MoveRight", this, &ASoftDesignTrainingPlayerController::MoveCameraRight);

    // Zoom camera
    InputComponent->BindAxis("Zoom", this, &ASoftDesignTrainingPlayerController::ZoomCamera);

    // Move character on click
    InputComponent->BindAction("MoveCharacter", EInputEvent::IE_Released, this, &ASoftDesignTrainingPlayerController::MoveCharacter);
    
    InputComponent->BindAction("Activate", EInputEvent::IE_Pressed, this, &ASoftDesignTrainingPlayerController::Activate);
    InputComponent->BindAction("Activate", EInputEvent::IE_Released, this, &ASoftDesignTrainingPlayerController::Deactivate);
}

void ASoftDesignTrainingPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Always show the mouse
    bShowMouseCursor = true;

    // Allow character to be moved by default 
    m_CanMoveCharacter = true;

    // In case we are activating a bridge
    m_BridgeActivated = nullptr;

    // In case we are activating a boat operator
    m_BoatOperatorActivated = nullptr;
}

void ASoftDesignTrainingPlayerController::MoveCameraForward(float value)
{
    ASoftDesignTrainingMainCharacter* character = Cast<ASoftDesignTrainingMainCharacter>(GetPawn());

    if (character)
    {
        character->MoveCameraForward(value);
    }
}

void ASoftDesignTrainingPlayerController::MoveCameraRight(float value)
{
    ASoftDesignTrainingMainCharacter* character = Cast<ASoftDesignTrainingMainCharacter>(GetPawn());

    if (character)
    {
        character->MoveCameraRight(value);
    }
}

void ASoftDesignTrainingPlayerController::ZoomCamera(float axisValue)
{
    ASoftDesignTrainingMainCharacter* character = Cast<ASoftDesignTrainingMainCharacter>(GetPawn());

    if (character)
    {
        character->ZoomCamera(axisValue);
    }
}


void ASoftDesignTrainingPlayerController::MoveCharacter()
{
    // TODO : find the position of the mouse in the world 
    // And move the agent to this position IF possible
    // Validate you can move through m_CanMoveCharacter
    if (!m_CanMoveCharacter)
    {
        return;
    }

    FVector world_pos, world_dir;

   DeprojectMousePositionToWorld(world_pos, world_dir);

   FHitResult HitResult;
   FVector Start = world_pos;
   FVector End = Start + (world_dir * 10000.f); 

   FCollisionQueryParams QueryParams;
   QueryParams.AddIgnoredActor(this); 

   
   if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
   {

       FVector HitLocation = HitResult.ImpactPoint;
       DrawDebugSphere(GetWorld(), HitLocation, 20, 20, FColor::Cyan, false, 2);

       auto* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
       if (NavSystem)
       {
           //project point on navmesh
           FNavLocation nav_location;
           if (NavSystem->ProjectPointToNavigation(HitLocation, nav_location))
           {
               UE_LOG(LogTemp, Warning, TEXT("Location is on the nav mesh"));
               UNavigationPath* path = NavSystem->FindPathToLocationSynchronously(GetWorld(), GetPawn()->GetActorLocation(), nav_location.Location);
               if (path && path->PathPoints.Num() > 0)
               {
                   // Affichage du chemin : sphères rouges sur les points et lignes jaunes entre eux
                   for (int32 i = 0; i < path->PathPoints.Num(); i++)
                   {
                       FVector Point = path->PathPoints[i];
                       DrawDebugSphere(GetWorld(), Point, 20, 20, FColor::Red, false, 2);

                       if (i > 0)
                       {
                           DrawDebugLine(GetWorld(), path->PathPoints[i - 1], Point, FColor::Yellow, false, 2, 0, 5.0f);
                       }
                   }

                    FAIMoveRequest move_request(HitLocation);
                    move_request.SetAcceptanceRadius(10);
                    m_PathFollowingComponent->RequestMove(move_request, path->GetPath());
               }
           }
       }
   }
}

void ASoftDesignTrainingPlayerController::Activate()
{
    APawn* pawn = GetPawn();
    if (pawn == nullptr)
    {
        return;
    }

    m_CanMoveCharacter = false;
    // TODO : Mouvement of the agent should be stopped !!

    // Make an overlap to find what is near us to activate it
    TArray<FOverlapResult> results;
    GetWorld()->OverlapMultiByChannel(results, pawn->GetActorLocation(), pawn->GetActorRotation().Quaternion(), ECollisionChannel::ECC_WorldDynamic, FCollisionShape::MakeSphere(200.f));

    for (FOverlapResult& result : results)
    {
        AActor* actor = result.GetActor();
        ASDTBridge* bridge = Cast<ASDTBridge>(actor);
        if (bridge)
        {
            bridge->Activate();
            m_BridgeActivated = bridge;
            break;
        }

        ASDTBoatOperator* boatOperator = Cast<ASDTBoatOperator>(actor);
        if (boatOperator)
        {
            boatOperator->Activate();
            m_BoatOperatorActivated = boatOperator;
            break;
        }
    }
}

void ASoftDesignTrainingPlayerController::Deactivate()
{
    m_CanMoveCharacter = true;

    if (m_BridgeActivated)
    {
        m_BridgeActivated->Deactivate();
    }
    if (m_BoatOperatorActivated)
    {
        m_BoatOperatorActivated->Deactivate();
    }
}