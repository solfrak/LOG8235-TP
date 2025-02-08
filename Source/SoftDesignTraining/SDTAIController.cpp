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
    float actor_current_speed = current_speed;

    actor_current_speed += m_acceleration * deltaTime;

    if (actor_current_speed > m_max_speed)
    {
        actor_current_speed = m_max_speed;
    }

    // Move the pawn
    if (!m_velocity.IsZero() && m_is_rotating) {
        forward_direction += m_velocity;
    }

    pawn->AddMovementInput(forward_direction, actor_current_speed);

    // Smooth rotation towards movement direction
    const FRotator current_rot = pawn->GetActorRotation();
    const FRotator target_rot = forward_direction.Rotation();
    const FRotator result = FMath::Lerp(current_rot, target_rot, 0.1f);
    pawn->SetActorRotation(result);
    //m_velocity = FVector::ZeroVector;
}

void ASDTAIController::GetObstacleDimension(float& center, float& hitPoint, AActor* collisionObstacle, FHitResult hit) {
    
    FVector localMin;
    FVector localMax;

    FTransform actorTransform = collisionObstacle->GetTransform();
    UStaticMeshComponent* hitComponent = collisionObstacle->FindComponentByClass<UStaticMeshComponent>();
    hitComponent->GetLocalBounds(localMin, localMax);
    FVector worldMin = actorTransform.TransformPosition(localMin);
    FVector worldMax = actorTransform.TransformPosition(localMax);

    if (hit.ImpactNormal.Y != 0) {
        center = (worldMin.X + worldMax.X) / 2;
        hitPoint = hit.Location.X;
    }
    else {
        center = (worldMin.Y + worldMax.Y) / 2;
        hitPoint = hit.Location.Y;
    }
}

void ASDTAIController::AvoidObstacle(FHitResult hit, float deltaTime) {
    
    PhysicsHelpers helper(GetWorld());

    APawn* pawn = GetPawn();
    FVector const pawnPosition(pawn->GetActorLocation());

    AStaticMeshActor* hitActor = Cast<AStaticMeshActor>(hit.GetActor());
    //m_current_transition_duration += deltaTime;
    if (hitActor != nullptr) {

        float center;
        float hitPoint;
        GetObstacleDimension(center, hitPoint, hitActor, hit);

        TArray<struct FHitResult> leftHits;
        TArray<struct FHitResult> rightHits;

        float rotationAngle = 45.0f;
        FVector rotationAxis(0.0f, 0.0f, 1.0f);
        helper.CastRay(pawnPosition, pawnPosition + pawn->GetActorRightVector() * m_wall_detection_distance * 1.5, leftHits, true);
        helper.CastRay(pawnPosition, pawnPosition + pawn->GetActorRightVector() * m_wall_detection_distance * 1.5, rightHits, true);

        FVector product = (FVector::CrossProduct(hit.ImpactNormal, pawn->GetActorUpVector()).GetSafeNormal()).GetSafeNormal() * m_rotation_strength;

        //un peu beaucoup de repetition
        if (hitPoint <= center) {
            if (leftHits.Num() == 0) {
                UE_LOG(LogTemp, Warning, TEXT("WE GO LEFT, LEFT"));
                m_velocity = -product;
            }
            else if (rightHits.Num() == 0) {
                UE_LOG(LogTemp, Warning, TEXT("WE GO LEFT, RIGHT"));
                m_velocity = product;
            }

            else {
                UE_LOG(LogTemp, Warning, TEXT("WE GO LEFT, DEFAULT"));
                m_velocity = product;
            }
        }
        else {
            if (rightHits.Num() == 0) {
                UE_LOG(LogTemp, Warning, TEXT("WE GO RIGHT, RIGHT"));
                m_velocity = product;
            }
            else if (leftHits.Num() == 0) {
                UE_LOG(LogTemp, Warning, TEXT("WE GO RIGHT, LEFT"));
                m_velocity = -product;
            }
            else {
                UE_LOG(LogTemp, Warning, TEXT("WE GO RIGHT, DEFAULT"));
                m_velocity = -product;
            }
        }
    }
}

void ASDTAIController::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
    FHitResult hit;
    if (DetectObstacle(hit) && !m_is_rotating) { //detecte un obstacle et tu ne tournes pas
        m_is_rotating = true;
        AvoidObstacle(hit, deltaTime); //on te fait tourner
    }
    else if (!DetectObstacle(hit) && m_is_rotating) {
        m_is_rotating = false;
        m_velocity = FVector::ZeroVector;
        m_current_transition_duration = 0;
    }
    else if (m_is_rotating && m_current_transition_duration < m_transition_duration) { //t'es encore en train de tourner
        m_current_transition_duration += deltaTime;
    }
    else {
        m_is_rotating = false;
        m_velocity = FVector::ZeroVector;
        m_current_transition_duration = 0;

    }
    DoMovement(deltaTime, GetPawn());

}

//pas utiliser pour le moment, on veut ajouter detection obstacle
bool ASDTAIController::MoveToTarget(FVector2D target, float speed, float deltaTime)
{

    FHitResult hit;
    if (DetectObstacle(hit)) {
        AvoidObstacle(hit, deltaTime);
    }
    return false;
}


bool ASDTAIController::DetectObstacle(FHitResult& hitResult)
{
    PhysicsHelpers helper(GetWorld());

    APawn* pawn = GetPawn();
    FVector const pawnPosition(pawn->GetActorLocation());
    TArray<FHitResult> hits;

    //faudra revoir pour ca parce qu'on met le collision channel ici mais jai hard coder dans physicshelper l'autre channel
    helper.CapsuleCast(pawnPosition, pawnPosition + pawn->GetActorForwardVector() * m_wall_detection_distance, m_capsule_radius, hits, COLLISION_DEATH_OBJECT, true);
    
    float distance = FLT_MAX;
    //debug purpose
    for (const FHitResult& hit : hits)
    {
        //need to cast and still check if death traps or walls? capsulecast should arealdy do that i think
        float hitDistance = (hit.ImpactPoint - pawnPosition).Size();
        if (hitDistance < distance) {
            hitResult = hit;
        }
    }
    return hits.Num() != 0;

}



bool ASDTAIController::DetectPickup(FVector& vector) 
{
    PhysicsHelpers helper(GetWorld());
    
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return false;
    }

    FVector PawnLocation = ControlledPawn->GetActorLocation();

    TArray<FOverlapResult> OverlapResults;
    bool bOverlapped = helper.SphereOverlap(PawnLocation + ControlledPawn->GetActorForwardVector() * m_pickup_detection_range, m_pickup_detection_radius, OverlapResults, COLLISION_COLLECTIBLE, true);
    
    for (const FOverlapResult& Result : OverlapResults)
    {
        AActor* OverlappedActor = Result.GetActor();
        if (!OverlappedActor)
        {
            continue;
        }

        ASDTCollectible* Pickup = Cast<ASDTCollectible>(OverlappedActor);
        if (Pickup && !Pickup->IsOnCooldown())
        {
            bool bObstacle = SDTUtils::Raycast(GetWorld(), PawnLocation, Pickup->GetActorLocation());
            
            if (!bObstacle) {
                vector = Pickup->GetActorLocation();
                return true;
            }
        }
    }

    return false;
}