// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavLinkCustomInterface.h"

#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{

}

/**
* This function is called every frame while the AI is following a path.
* MoveSegmentStartIndex and MoveSegmentEndIndex specify where we are on the path point array.
*/
void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    if (!Path.IsValid() || Path->GetPathPoints().Num() == 0)
    {
        return;
    }

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
    const FNavPathPoint& segmentEnd = points[MoveSegmentEndIndex];

    APawn* ControlledPawn = Cast<APawn>(GetOwner());
    if (!ControlledPawn)
    {
        return;
    }

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        // Update jump along path / nav link proxy
    }
    else
    {
        // Update navigation along path (move along)
        FVector currentLocation = ControlledPawn->GetActorLocation();
        FVector direction = (segmentEnd.Location - currentLocation).GetSafeNormal();
        float speed = ControlledPawn->GetMovementComponent()->GetMaxSpeed();
        FVector newLocation = currentLocation + direction * speed * DeltaTime;

        ControlledPawn->SetActorLocation(newLocation);

        // Si on est proche du point d'arrivée, passe au segment suivant
        const float Tolerance = 50.f; // distance de tolérance (à ajuster)
        if (FVector::Dist(newLocation, segmentEnd.Location) < Tolerance)
        {
            SetMoveSegment(MoveSegmentEndIndex);
        }
    }
}

/**
* This function is called every time the AI has reached a new point on the path.
* If you need to do something at a given point in the path, this is the place.
*/
void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    Super::SetMoveSegment(segmentStartIndex);

    if (!Path.IsValid() || Path->GetPathPoints().Num() == 0)
    {
        return;
    }

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();

    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

    if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
        // Handle starting jump
    }
    else
    {
        // Handle normal segments
        isJumping = false;
        jumProgress = 0.f;
    }
}

