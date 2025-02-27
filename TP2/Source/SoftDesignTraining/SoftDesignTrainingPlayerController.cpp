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
    // Vérifie d’abord si on autorise le déplacement
    if (!m_CanMoveCharacter)
        return;

    FHitResult hitResult;
    // ECC_Visibility ou un autre canal selon ta config
    bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, hitResult);

    if (bHit && hitResult.bBlockingHit)
    {
        // Ici, on a la position de la souris dans le monde
        FVector targetLocation = hitResult.ImpactPoint;

        APawn* ControlledPawn = GetPawn();
        if (!ControlledPawn)
            return;

        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
        if (NavSys)
        {
            UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(GetWorld(), ControlledPawn->GetActorLocation(), targetLocation);
            if (NavPath && NavPath->IsValid())
            {
                // Affecte le chemin calculé à ton composant personnalisé
                // Ici, on suppose que ton composant possède une méthode pour recevoir le chemin,
                // par exemple SetPath(NavPath) ou une fonction équivalente.
                m_PathFollowingComponent->SetPath(NavPath);

                // Si nécessaire, démarre le suivi du chemin (cela peut se faire dans le composant ou ici)
                // m_PathFollowingComponent->StartPathFollowing(); // méthode à implémenter si besoin
            }
        }

        // Ensuite, on peut afficher le chemin (debug draw) si on veut
        DrawDebugPath();
    }
}

void ASoftDesignTrainingPlayerController::DrawDebugPath()
{
    if (!m_PathFollowingComponent)
        return;

    // Récupère le path
    FNavPathSharedPtr navPath = m_PathFollowingComponent->GetPath();
    if (!navPath.IsValid())
        return;

    const TArray<FNavPathPoint>& pathPoints = navPath->GetPathPoints();
    // On dessine un trait entre chaque point
    for (int i = 0; i < pathPoints.Num() - 1; ++i)
    {
        DrawDebugPoint(GetWorld(), pathPoints[i].Location, 10.0, FColor::Red, false, 2.0f);
        DrawDebugLine(
            GetWorld(),
            pathPoints[i].Location,
            pathPoints[i + 1].Location,
            FColor::Green,
            false,  // bPersistentLines (false = disparaît au bout de quelques secondes)
            2.0f,   // Durée d'affichage
            0,      // DepthPriority
            5.0f    // Épaisseur
        );
    }

    DrawDebugPoint(GetWorld(), pathPoints[pathPoints.Num() - 1].Location, 10.0, FColor::Red, false, 2.0f);
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