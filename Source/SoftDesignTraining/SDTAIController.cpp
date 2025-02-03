// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "DrawDebugHelpers.h"


ASDTAIController::ASDTAIController()
{
    //Variables
    Acceleration = 200.f;
    MaxSpeed = 600.f;
    MovementDirection = FVector::ZeroVector;
    Velocity = FVector::ZeroVector;
}

FVector ASDTAIController::GetMovementDirection() const
{
    APawn* ControlledPawn = GetPawn();
    if (MovementDirection.IsNearlyZero())
    {
        if (ControlledPawn)
            return ControlledPawn->GetActorForwardVector();
        else
            return FVector::ForwardVector;

    }
    return MovementDirection.GetSafeNormal();
}

void ASDTAIController::UpdateVelocity(float deltaTime)
{
    //calcule de vélocité
    FVector Direction = GetMovementDirection();
    Velocity += Direction * Acceleration * deltaTime;

    //set max value sur velocity
    if (Velocity.Size() > MaxSpeed)
    {
        Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
    }
}

void ASDTAIController::ApplyMovement()
{
    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn)
    {
      
        ControlledPawn->AddMovementInput(GetMovementDirection(), Velocity.Size());
    }
}

void ASDTAIController::ApplyRotation()
{
    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn && !Velocity.IsNearlyZero())
    {
        //convert speed vect to rotation
        FRotator TargetRotation = Velocity.Rotation();
        FHitResult SweepHit;

        ControlledPawn->AddActorWorldRotation(TargetRotation, true, &SweepHit, ETeleportType::None);
    }

}

void ASDTAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    //update speed vector
    UpdateVelocity(deltaTime);
    //apply movement on pawn
    ApplyMovement();
    //Orient pawwn to direction
    ApplyRotation();

    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn)
    {
        DrawDebugLine(GetWorld(), ControlledPawn->GetActorLocation(), 
            ControlledPawn->GetActorLocation() + Velocity, FColor::Green, false, 0.1f, 0, 2.0f);
    }

}