//// Fill out your copyright notice in the Description page of Project Settings.
//
//#include "SDTPathFollowingComponent.h"
//#include "SoftDesignTraining.h"
//#include "SDTUtils.h"
//#include "SDTAIController.h"
//#include "GameFramework/CharacterMovementComponent.h"
//#include "NavigationSystem.h"
//#include "NavLinkCustomInterface.h"
//#include "SDTAnimNotify_JumpStart.h"
//#include "SoftDesignTrainingMainCharacter.h"
//
//
//#include "DrawDebugHelpers.h"
//
//USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
//{
//
//}
//
//void GenerateArcPath(const FVector& Start, const FVector& End, float ArcHeight, TArray<FVector>& OutPathPoints, int NumPoints = 20)
//{
//    OutPathPoints.Empty();
//
//    // Define the control point (midpoint, elevated)
//    FVector Control = (Start + End) * 0.5f;
//    Control.Z += ArcHeight; // Raise control point for the arc
//
//    for (int i = 0; i <= NumPoints; i++)
//    {
//        float t = static_cast<float>(i) / NumPoints;
//
//        // Quadratic Bézier interpolation
//        FVector Point =
//            (1 - t) * (1 - t) * Start +
//            2 * (1 - t) * t * Control +
//            t * t * End;
//
//        OutPathPoints.Add(Point);
//    }
//}
//
///**
//* This function is called every frame while the AI is following a path.
//* MoveSegmentStartIndex and MoveSegmentEndIndex specify where we are on the path point array.
//*/
//void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
//{
//
//    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
//    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
//    const FNavPathPoint& segmentEnd = points[MoveSegmentEndIndex];
//
//    if (SDTUtils::HasJumpFlag(segmentStart))
//    {
//        if (!isJumping)
//        {
//            // Force the notify to trigger manually for debugging
//            ACharacter* Character = Cast<ACharacter>(MovementComp->GetOwner());
//
//            if (Character && Character->GetMesh() && Character->GetMesh()->GetAnimInstance())
//            {
//                USDTAnimNotify_JumpStart* JumpNotify = NewObject<USDTAnimNotify_JumpStart>();
//                JumpNotify->Notify(Character->GetMesh(), nullptr);
//            }
//            // Generate arc path
//            jumpTrajectoryArray.Empty();
//            GenerateArcPath(segmentStart.Location, segmentEnd.Location, 1000.0f, jumpTrajectoryArray, 60);
//            jumProgress = 0.0f;
//            isJumping = true;
//
//            UE_LOG(LogTemp, Warning, TEXT("Jump Triggered from PathFollowingComponent!"));
//        }
//
//        // Move along arc
//        float JumpSpeed = 1;
//        jumProgress = FMath::Clamp(jumProgress + (DeltaTime * JumpSpeed), 0.0f, 1.0f);
//
//        {
//            ACharacter* OwnerCharacter = Cast<ACharacter>(MovementComp->GetOwner());
//            if (OwnerCharacter)
//            {
//                ASoftDesignTrainingMainCharacter* MainChar = Cast<ASoftDesignTrainingMainCharacter>(OwnerCharacter);
//                if (MainChar)
//                {
//                    MainChar->isJumping = isJumping;
//                }
//            }
//        }
//
//        int32 SegmentIndex = FMath::FloorToInt(jumProgress * (jumpTrajectoryArray.Num() - 1));
//        float Alpha = FMath::Frac(jumProgress * (jumpTrajectoryArray.Num() - 1));
//
//        FVector NewLocation = FMath::Lerp(
//            jumpTrajectoryArray[SegmentIndex],
//            jumpTrajectoryArray[FMath::Min(SegmentIndex + 1, jumpTrajectoryArray.Num() - 1)],
//            Alpha
//        );
//
//        MovementComp->GetOwner()->SetActorLocation(NewLocation);
//
//        // End jump when completed
//        if (jumProgress >= 1.0f)
//        {
//            {
//                ACharacter* OwnerCharacter = Cast<ACharacter>(MovementComp->GetOwner());
//                if (OwnerCharacter)
//                {
//                    ASoftDesignTrainingMainCharacter* MainChar = Cast<ASoftDesignTrainingMainCharacter>(OwnerCharacter);
//                    if (MainChar)
//                    {
//                        MainChar->isJumping = false;
//                    }
//                }
//            }
//            isJumping = false;
//            SetMoveSegment(MoveSegmentEndIndex); 
//            OnSegmentFinished();
//        }
//
//    }
//    else
//    {
//        const FVector CurrentLocation = MovementComp->GetActorFeetLocation();
//        const FVector CurrentTarget = GetCurrentTargetLocation();
//        CurrentMoveInput = (CurrentTarget - CurrentLocation).GetSafeNormal();
//        MovementComp->RequestPathMove(CurrentMoveInput);
//
//    }
//}
//
///**
//* This function is called every time the AI has reached a new point on the path.
//* If you need to do something at a given point in the path, this is the place.
//*/
//void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
//{
//
//    //Super::SetMoveSegment(segmentStartIndex);
//    MoveSegmentStartIndex = segmentStartIndex;
//    MoveSegmentEndIndex = segmentStartIndex + 1;
//	const FNavigationPath* PathInstance = Path.Get();
//	const FVector SegmentStart = *PathInstance->GetPathPointLocation(MoveSegmentStartIndex);
//	CurrentDestination = PathInstance->GetPathPointLocation(MoveSegmentEndIndex);
//	FVector SegmentEnd = *CurrentDestination;
//
//
//	MoveSegmentDirection = (SegmentEnd - SegmentStart).GetSafeNormal();
//
//	UpdateMoveFocus();
//	UpdateDecelerationData();
//
//    isJumping = false;        //Reset jump flag explicitly here
//    jumProgress = 0.0f;
//
//    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
//
//    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
//
//    if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
//    {
//        // Handle starting jump
//        Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Flying);
//    }
//    else
//    {
//        Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Walking);
//    }
//}
//
//void USDTPathFollowingComponent::SetIsJumping(bool bJumping)
//{
//    isJumping = bJumping;
//    UE_LOG(LogTemp, Warning, TEXT("AI Jump State Updated: %s"), isJumping ? TEXT("Jumping") : TEXT("Not Jumping"));
//}
// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "NavLinkCustomInterface.h"
#include "SDTAnimNotify_JumpStart.h"
#include "SDTAnimNotify_JumpEnd.h"
#include "SoftDesignTrainingMainCharacter.h"  // For main character access
#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{

}

void GenerateArcPath(const FVector& Start, const FVector& End, float ArcHeight, TArray<FVector>& OutPathPoints, int NumPoints /*= 20*/)
{
    OutPathPoints.Empty();

    // Calculate the elevated control point (midpoint of the arc)
    FVector Control = (Start + End) * 0.5f;
    Control.Z += ArcHeight;

    for (int i = 0; i <= NumPoints; i++)
    {
        float t = static_cast<float>(i) / NumPoints;
        // Quadratic Bézier interpolation for arc path
        FVector Point =
            (1 - t) * (1 - t) * Start +
            2 * (1 - t) * t * Control +
            t * t * End;
        OutPathPoints.Add(Point);
    }
}

/**
 * Called every frame while the AI follows a path.
 * MoveSegmentStartIndex and MoveSegmentEndIndex indicate our current segment.
 */
void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStart = points[MoveSegmentStartIndex];
    const FNavPathPoint& segmentEnd = points[MoveSegmentEndIndex];

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        if (!isJumping)
        {
            // Trigger JumpStart notify
            ACharacter* Character = Cast<ACharacter>(MovementComp->GetOwner());
            if (Character && Character->GetMesh() && Character->GetMesh()->GetAnimInstance())
            {
                USDTAnimNotify_JumpStart* JumpNotify = NewObject<USDTAnimNotify_JumpStart>();
                JumpNotify->Notify(Character->GetMesh(), nullptr);
            }
            // Generate the jump arc
            jumpTrajectoryArray.Empty();
            GenerateArcPath(segmentStart.Location, segmentEnd.Location, 1000.0f, jumpTrajectoryArray, 60);
            jumProgress = 0.0f;
            isJumping = true;
            UE_LOG(LogTemp, Warning, TEXT("Jump Triggered from PathFollowingComponent!"));
        }

        // Update jump progress
        float JumpSpeed = 1.0f;
        jumProgress = FMath::Clamp(jumProgress + (DeltaTime * JumpSpeed), 0.0f, 1.0f);

        // Update the owning main character's jump flag
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(MovementComp->GetOwner()))
        {
            if (ASoftDesignTrainingMainCharacter* MainChar = Cast<ASoftDesignTrainingMainCharacter>(OwnerCharacter))
            {
                MainChar->BIsJumping = isJumping;
            }
        }

        // Compute new location along the arc
        int32 SegmentIndex = FMath::FloorToInt(jumProgress * (jumpTrajectoryArray.Num() - 1));
        float Alpha = FMath::Frac(jumProgress * (jumpTrajectoryArray.Num() - 1));
        FVector NewLocation = FMath::Lerp(
            jumpTrajectoryArray[SegmentIndex],
            jumpTrajectoryArray[FMath::Min(SegmentIndex + 1, jumpTrajectoryArray.Num() - 1)],
            Alpha
        );

        // Orient the character to face the jump trajectory (yaw only)
        int32 NextIndex = FMath::Min(SegmentIndex + 1, jumpTrajectoryArray.Num() - 1);
        FVector CurrentPoint = jumpTrajectoryArray[SegmentIndex];
        FVector NextPoint = jumpTrajectoryArray[NextIndex];
        FVector DesiredDirection = (NextPoint - CurrentPoint).GetSafeNormal();
        // Zero out vertical component so only yaw changes
        DesiredDirection.Z = 0.0f;
        DesiredDirection.Normalize();
        FRotator DesiredRotation = DesiredDirection.Rotation();

        AActor* OwnerActor = MovementComp->GetOwner();
        FRotator CurrentRotation = OwnerActor->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DesiredRotation, DeltaTime, 10.0f);
        OwnerActor->SetActorRotation(NewRotation);

        MovementComp->GetOwner()->SetActorLocation(NewLocation);

        if (jumProgress >= 1.0f)
        {
            if (ACharacter* OwnerCharacter = Cast<ACharacter>(MovementComp->GetOwner()))
            {
                if (ASoftDesignTrainingMainCharacter* MainChar = Cast<ASoftDesignTrainingMainCharacter>(OwnerCharacter))
                {
                    MainChar->BIsJumping = false;
                }
                else if (OwnerCharacter->GetMesh() && OwnerCharacter->GetMesh()->GetAnimInstance())
                {
                    USDTAnimNotify_JumpEnd* JumpEndNotify = NewObject<USDTAnimNotify_JumpEnd>();
                    JumpEndNotify->Notify(OwnerCharacter->GetMesh(), nullptr);
                }
            }
            isJumping = false;
            SetMoveSegment(MoveSegmentEndIndex);
            OnSegmentFinished();
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
 * Called when the AI reaches a new path point.
 */
void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    MoveSegmentStartIndex = segmentStartIndex;
    MoveSegmentEndIndex = segmentStartIndex + 1;

    const FNavigationPath* PathInstance = Path.Get();
    const FVector SegmentStart = *PathInstance->GetPathPointLocation(MoveSegmentStartIndex);
    CurrentDestination = PathInstance->GetPathPointLocation(MoveSegmentEndIndex);
    FVector SegmentEnd = *CurrentDestination;
    MoveSegmentDirection = (SegmentEnd - SegmentStart).GetSafeNormal();

    UpdateMoveFocus();
    UpdateDecelerationData();

    // Reset jump state for the new segment
    isJumping = false;
    jumProgress = 0.0f;

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    const FNavPathPoint& segmentStartPoint = points[MoveSegmentStartIndex];

    if (SDTUtils::HasJumpFlag(segmentStartPoint) && FNavMeshNodeFlags(segmentStartPoint.Flags).IsNavLink())
    {
        Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Flying);
    }
    else
    {
        Cast<UCharacterMovementComponent>(MovementComp)->SetMovementMode(MOVE_Walking);
    }
}

void USDTPathFollowingComponent::SetIsJumping(bool bJumping)
{
    isJumping = bJumping;
    UE_LOG(LogTemp, Warning, TEXT("AI Jump State Updated: %s"), isJumping ? TEXT("Jumping") : TEXT("Not Jumping"));
}
