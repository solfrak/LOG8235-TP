// Fill out your copyright notice in the Description page of Project Settings.
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"
#include "SDTUtils.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "SDTCollectible.h"


void ASDTAIController::DoMovement(float deltaTime, APawn* pawn)
{
    // Apply a forward acceleration until max speed is reached
    FVector forward_direction = pawn->GetActorForwardVector();
    float current_speed = m_velocity.Size();
    
    if (current_speed < m_max_speed) 
    {
        m_velocity += forward_direction * m_acceleration * deltaTime;
    }

    // Clamp velocity to max speed
    m_velocity = m_velocity.GetClampedToMaxSize(m_max_speed);
    m_velocity.Z = 0.0f;

    // Move the pawn
    pawn->AddMovementInput(m_velocity.GetSafeNormal(), 1.0f);

    // Smooth rotation towards movement direction
    const FRotator current_rot = pawn->GetActorRotation();
    const FRotator target_rot = m_velocity.Rotation();
    const FRotator result = FMath::Lerp(current_rot, target_rot, 0.05f);
    pawn->SetActorRotation(result);
}

void ASDTAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
    APawn *pawn = GetPawn();

    //Detect Wall
    auto hits = GetObstacleInPath();
    for (auto hit : hits)
    {
        m_velocity += hit.ImpactNormal * (1.0f / hit.Distance) * 50;
    }

    //Check for collectable
    MoveTowardCollectable();
    

    DoMovement(deltaTime, pawn);
}

TArray<FHitResult> ASDTAIController::GetObstacleInPath()
{
    APawn *pawn = GetPawn();
    FVector current_pos = pawn->GetActorLocation();
    TArray<FHitResult> results;
    
    //TODO: use physics helper to make a sweep
    PhysicsHelpers Helpers(GetWorld());
    Helpers.CapsuleCast(current_pos, current_pos + pawn->GetActorForwardVector() * 200, 30, results, ECC_Visibility, true);

    return results;
}

void ASDTAIController::MoveTowardCollectable()
{
    APawn *pawn = GetPawn();
    PhysicsHelpers Helpers(GetWorld());

    TArray<FOverlapResult> results;
    Helpers.SphereOverlap(pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200, 200, results, COLLISION_COLLECTIBLE, true);

    if (results.Num() > 0) 
    {
        // Get first collectible position
        FVector targetLocation = results[0].GetActor()->GetActorLocation();
        FVector direction = (targetLocation - pawn->GetActorLocation()).GetSafeNormal();

        // Lock movement to 2D
        direction.Z = 0.0f;

        // Apply force to steer toward the collectible
        m_velocity += direction * m_acceleration;

        // Clamp velocity to max speed
        m_velocity = m_velocity.GetClampedToMaxSize(m_max_speed);
    }
}

