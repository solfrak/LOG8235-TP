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

void GenerateArcPath(const FVector& Start, const FVector& End, float ArcHeight, TArray<FVector>& OutPathPoints, int NumPoints = 20)
{
    OutPathPoints.Empty();

    // Define the control point (midpoint, elevated)
    FVector Control = (Start + End) * 0.5f;
    Control.Z += ArcHeight; // Raise control point for the arc

    for (int i = 0; i <= NumPoints; i++)
    {
        float t = static_cast<float>(i) / NumPoints;

        // Quadratic Bézier interpolation
        FVector Point =
            (1 - t) * (1 - t) * Start +
            2 * (1 - t) * t * Control +
            t * t * End;

        OutPathPoints.Add(Point);
    }
}

/**
* This function is called every frame while the AI is following a path.
* MoveSegmentStartIndex and MoveSegmentEndIndex specify where we are on the path point array.
*/
void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
    const FNavPathPoint& segmentEnd = points[MoveSegmentEndIndex];

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        // Update jump along path / nav link proxy
        // Handle jumping logic
        //if (ACharacter* Character = Cast<ACharacter>(MovementComp->GetOwner()))
        //{
        //    FVector JumpVelocity = (segmentEnd.Location - segmentStart.Location);
        //    JumpVelocity.Z = 10.0f; // Ensure the jump has an upward component
        //
        //    Character->LaunchCharacter(JumpVelocity, true, true);
        //}


        if (!isJumping)
        {
            // Generate arc path
            jumpTrajectoryArray.Empty();
            GenerateArcPath(segmentStart.Location, segmentEnd.Location, 1000.0f, jumpTrajectoryArray, 60);
            jumProgress = 0.0f;
            isJumping = true;
        }

        // Move along arc
        float JumpSpeed = 1;
        jumProgress = FMath::Clamp(jumProgress + (DeltaTime * JumpSpeed), 0.0f, 1.0f);
        int32 SegmentIndex = jumProgress * (jumpTrajectoryArray.Num() - 1);
        float Alpha = FMath::Frac(jumProgress * (jumpTrajectoryArray.Num() - 1));

        FVector NewLocation = FMath::Lerp(
            jumpTrajectoryArray[SegmentIndex],
            jumpTrajectoryArray[FMath::Min(SegmentIndex + 1, jumpTrajectoryArray.Num() - 1)],
            Alpha
        );

        MovementComp->GetOwner()->SetActorLocation(NewLocation);

        // End jump when completed
        if (jumProgress >= 1.0f)
        {
            isJumping = false;
        }

    }
    else
    {
        const FVector CurrentLocation = MovementComp->GetActorFeetLocation();
        const FVector CurrentTarget = GetCurrentTargetLocation();
        CurrentMoveInput = (CurrentTarget - CurrentLocation).GetSafeNormal();
        MovementComp->RequestPathMove(CurrentMoveInput);

    }
}

/**
* This function is called every time the AI has reached a new point on the path.
* If you need to do something at a given point in the path, this is the place.
*/
void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{

    Super::SetMoveSegment(segmentStartIndex);
    UE_LOG(LogTemp, Warning, TEXT("Segment index %d"), segmentStartIndex);

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();

    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];

    if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
        // Handle starting jump
        //Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Flying);
    }
    else
    {
        //Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Walking);
    }
}

