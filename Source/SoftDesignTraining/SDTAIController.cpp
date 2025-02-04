// Fill out your copyright notice in the Description page of Project Settings.
#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"
#include "SDTUtils.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"
#include "SDTCollectible.h"


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

    DetectAndCollectPickup();

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

void ASDTAIController::DetectAndCollectPickup() 
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    //variables for detection range
    FVector DetectionRangeStart = ControlledPawn->GetActorLocation();
    FVector DetectionRangeForward = ControlledPawn->GetActorForwardVector();
    FVector DetectionRangeEnd = DetectionRangeStart + (DetectionRangeForward * PickupDetectionRange);

    //collision query and store obj that will be hit
    FHitResult Hit;
    FCollisionQueryParams QueryParams(FName(TEXT("PickupTrace")), true);
    QueryParams.AddIgnoredActor(ControlledPawn); //ignore self for trace

    //raycast with ECC_Visibility because we want to hit actors that are visible inside detection range
    if (GetWorld()->LineTraceSingleByChannel(Hit, DetectionRangeStart, DetectionRangeEnd, ECC_Visibility, QueryParams))
    {
        UE_LOG(LogTemp, Warning, TEXT("LineTrace hit: %s"), *Hit.GetActor()->GetName());//UE log to see what line hit
        ASDTCollectible* CollectiblePtr = Cast<ASDTCollectible>(Hit.GetActor());

        //if cast is Collectible and it's not on cooldown
        if (CollectiblePtr && !CollectiblePtr->IsOnCooldown())
        {
            FVector CollectiblePickupLocation = CollectiblePtr->GetActorLocation();
            //check if anything between AI and pickup. SDTUtils::Raycast to return true if anything between 
            bool bRayBlocked = SDTUtils::Raycast(GetWorld(), DetectionRangeStart, CollectiblePickupLocation);
            if (!bRayBlocked)
            {
                UE_LOG(LogTemp, Warning, TEXT("Raycast blocked: %s"), bRayBlocked ? TEXT("true") : TEXT("false"));
                //if nothing between, point AI direction to pickup. B - A, (location - rangeStart) to get vector for AI
                MovementDirection = (CollectiblePickupLocation - DetectionRangeStart).GetSafeNormal();
                UE_LOG(LogTemp, Warning, TEXT("Updated MovementDirection: %s"), *MovementDirection.ToString());
                //TODO change speed?

                //if pickup within threshold, collect it
                //FVector::Dist to get distance between 2 vector
                if (FVector::Dist(CollectiblePickupLocation, DetectionRangeStart) < PickupCollectionThreshold)
                {
                    CollectiblePtr->Collect();
                }
            }
        }
        DrawDebugCircle(
            GetWorld(),                
            DetectionRangeStart,
            PickupDetectionRange,      
            32,                        
            FColor::Blue,              
            false,                     
            0.1f,                      // LifeTime: how long the circle stays on screen.
            0,                         // Depth priority.
            2.0f,                      // Thickness of the circle line.
            FVector(1, 0, 0),          // XAxis: defines one axis for the circle's plane.
            FVector(0, 1, 0),          // YAxis: defines the other axis.
            false                      // bDrawAxis: set to false unless you want to visualize the circle's axes.
        );
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("LineTrace did not hit any actor."));
    }
}