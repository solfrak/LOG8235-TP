// Fill out your copyright notice in the Description page of Project Settings.
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"
#include "SDTUtils.h"
#include "Engine/StaticMeshActor.h"
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

void ASDTAIController::AvoidObstacle(float deltaTime) {
	//a fixer ici because ai is too stupid and goes in circle sometimes
	m_current_transition_duration += deltaTime;
	float ratio = deltaTime / m_transition_duration;
	
	APawn* pawn = GetPawn();
	FVector const pawnPosition(pawn->GetActorLocation());
	FVector const forward(pawn->GetActorForwardVector());
	FVector const side(pawn->GetActorRightVector()); //jutilise side ici (basically un 90 degre), mais no idea sil faut un certain angle selon l'enonce du tp

	FVector const newDir = FMath::Lerp(forward, side, ratio * m_rotation_speed);
	FVector newLocation = pawnPosition + FVector(newDir).GetSafeNormal() * m_max_speed * deltaTime;
	pawn->SetActorLocation(newLocation, true);
	pawn->SetActorRotation((newDir).ToOrientationQuat());
	//pawn->SetActorRotation(FVector(pawnPosition + FVector(newDir) * m_max_speed), 0.0f).ToOrientationQuat();

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

    //bool wall_detected = DetectWall(m_wall_detection_distance);
    //if (wall_detected) {
    //    AvoidObstacle(deltaTime);
    //}
    //else {
    //    m_current_transition_duration = 0.0f;
    //    MoveToTarget(FVector2D(GetPawn()->GetActorLocation() + GetPawn()->GetActorForwardVector() * m_wall_detection_distance), m_max_speed, deltaTime);
    //}
}

bool ASDTAIController::MoveToTarget(FVector2D target, float speed, float deltaTime)
{
    APawn* pawn = GetPawn();
    FVector const pawnPosition(pawn->GetActorLocation());
    FVector2D const toTarget = target - FVector2D(pawnPosition);
    //calcul de la vitesse ici
    FVector2D const displacement = FMath::Min(toTarget.Size(), speed * deltaTime) * toTarget.GetSafeNormal();
    pawn->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);
    //orientation
    pawn->SetActorRotation(FVector(displacement, 0.f).ToOrientationQuat());
    return toTarget.Size() < speed * deltaTime;
}


bool ASDTAIController::DetectWall(float distance)
{
    PhysicsHelpers helper(GetWorld());

    APawn* pawn = GetPawn();
    FVector const pawnPosition(pawn->GetActorLocation());

    TArray<FOverlapResult> results;
    //faudra revoir pour ca parce qu'on met le collision channel ici mais jai hard coder dans physicshelper l'autre channel
    helper.SphereOverlap(pawnPosition + pawn->GetActorForwardVector() * distance, m_radius_detection, results, COLLISION_DEATH_OBJECT, true);


    //debugging purposes
    for (const FOverlapResult& overlapResult : results)
    {
        if (overlapResult.GetActor()->ActorHasTag(FName("Death")))
        {
            UE_LOG(LogTemp, Warning, TEXT("WE DETECT A DEATH TRAP"));

        }
        else if (Cast<AStaticMeshActor>(overlapResult.GetActor()) != nullptr) {
            UE_LOG(LogTemp, Warning, TEXT("WE DETECT A WALL"));
        }

    }
    return results.Num() != 0;
}