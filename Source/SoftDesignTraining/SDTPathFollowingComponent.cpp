// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer& ObjectInitializer)
{

}


void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    if (!Path.IsValid() || MovementComp == nullptr)
    {
        return;
    }

    const TArray<FNavPathPoint>& points = Path->GetPathPoints();
    if (!points.IsValidIndex(MoveSegmentStartIndex))
    {
        return;
    }

    ASDTAIController* controller = Cast<ASDTAIController>(GetOwner());
    if (!controller) { return; }

    ACharacter* character = Cast<ACharacter>(controller->GetPawn());
    if (!character) { return; }

    UCharacterMovementComponent* charMoveComp = Cast<UCharacterMovementComponent>(character->GetMovementComponent());
    if (!charMoveComp) { return; }

    const FVector CurrentLocation = MovementComp->GetActorFeetLocation();
    const FVector CurrentTarget = GetCurrentTargetLocation();
    const FNavPathPoint& SegmentStart = points[MoveSegmentStartIndex];
    bool bHasJumpFlag = SDTUtils::HasJumpFlag(SegmentStart);

    if (bHasJumpFlag)
    {
        if (controller->InAir && !controller->Landing)
        {
            float EstimatedJumpDuration = 1.0f;
            if (EstimatedJumpDuration <= 0.f) EstimatedJumpDuration = 0.1f;

            m_JumpProgressRatio = FMath::Clamp(m_JumpProgressRatio + (DeltaTime / EstimatedJumpDuration), 0.0f, 1.0f);

            FVector nextLocation = FMath::Lerp(SegmentStart.Location, CurrentTarget, m_JumpProgressRatio);

            if (controller->JumpCurve)
            {
                float curveTime = FMath::Clamp(m_JumpProgressRatio, 0.f, 1.f);
                float jumpHeight = controller->JumpCurve->GetFloatValue(curveTime) * controller->JumpApexHeight;
                nextLocation += FVector(0.f, 0.f, jumpHeight);
            }

            FVector desiredVelocity = (nextLocation - character->GetActorLocation()) / DeltaTime;

            MovementComp->RequestDirectMove(desiredVelocity, true);

            if (m_JumpProgressRatio >= 1.0f)
            {
                controller->Landing = true;
                controller->InAir = false;
            }
        }
        else if (controller->Landing)
        {
            FVector ToTarget = (CurrentTarget - character->GetActorLocation());
            ToTarget.Z = 0;
            float DistToTargetSq = ToTarget.SizeSquared();

            const float TargetReachedThresholdSq = FMath::Square(50.0f);

            if (DistToTargetSq < TargetReachedThresholdSq)
            {
                controller->Landing = false;
                controller->AtJumpSegment = false;
                charMoveComp->SetMovementMode(MOVE_Walking);

                OnSegmentFinished();
            }
            else
            {
                FVector finalVelocity = ToTarget.GetSafeNormal() * charMoveComp->MaxWalkSpeed;
                MovementComp->RequestDirectMove(finalVelocity, false);
            }
        }
        else
        {
            //Fallback case  do nothing 
        }
        return;
    }

    bIsDecelerating = false;
    const bool bAccelerationBased = MovementComp->UseAccelerationForPathFollowing();
    if (bAccelerationBased)
    {
        CurrentMoveInput = (CurrentTarget - CurrentLocation).GetSafeNormal();

        if (MoveSegmentStartIndex >= DecelerationSegmentIndex)
        {
            const FVector PathEnd = Path->GetEndLocation();
            const float DistToEndSq = FVector::DistSquared(CurrentLocation, PathEnd);
            const bool bShouldDecelerate = DistToEndSq < FMath::Square(CachedBrakingDistance);
            if (bShouldDecelerate)
            {
                bIsDecelerating = true;

                const float SpeedPct = FMath::Clamp(FMath::Sqrt(DistToEndSq) / CachedBrakingDistance, 0.0f, 1.0f);
                CurrentMoveInput *= SpeedPct;
            }
        }

        PostProcessMove.ExecuteIfBound(this, CurrentMoveInput);
        MovementComp->RequestPathMove(CurrentMoveInput);
    }
    else
    {
        FVector MoveVelocity = (CurrentTarget - CurrentLocation) / DeltaTime;

        const int32 LastSegmentStartIndex = points.Num() - 2;
        const bool bNotFollowingLastSegment = (MoveSegmentStartIndex < LastSegmentStartIndex);

        PostProcessMove.ExecuteIfBound(this, MoveVelocity);
        MovementComp->RequestDirectMove(MoveVelocity, bNotFollowingLastSegment);
    }
}


void USDTPathFollowingComponent::SetMoveSegment(int32 SegmentStartIndex)
{
    Super::SetMoveSegment(SegmentStartIndex);

    if (!Path.IsValid())
    {
        return;
    }
    const TArray<FNavPathPoint>& points = Path->GetPathPoints();

    if (!points.IsValidIndex(SegmentStartIndex))
    {
        return;
    }

    ASDTAIController* controller = Cast<ASDTAIController>(GetOwner());
    if (!controller) { return; }

    ACharacter* character = Cast<ACharacter>(controller->GetPawn());
    if (!character) { return; }

    UCharacterMovementComponent* charMoveComp = Cast<UCharacterMovementComponent>(character->GetMovementComponent());
    if (!charMoveComp) { return; }

    const FNavPathPoint& SegmentStart = points[MoveSegmentStartIndex];
    bool bHasJumpFlag = SDTUtils::HasJumpFlag(SegmentStart);

    m_JumpProgressRatio = 0.f;

    if (bHasJumpFlag)
    {
        controller->AtJumpSegment = true;
        controller->InAir = true;
        controller->Landing = false;

        MovementComp->StopMovementKeepPathing();

        charMoveComp->bOrientRotationToMovement = false;
        charMoveComp->SetMovementMode(MOVE_Flying);
    }
    else
    {
        if (controller->AtJumpSegment || controller->InAir || controller->Landing)
        {
            controller->GetPawn()->bUseControllerRotationYaw = false;
            controller->AtJumpSegment = false;
            controller->InAir = false;
            controller->Landing = false;

            charMoveComp->bOrientRotationToMovement = true;

            if (charMoveComp->MovementMode != MOVE_Walking && charMoveComp->MovementMode != MOVE_NavWalking)
            {
                charMoveComp->SetMovementMode(MOVE_Walking);
            }
        }
        else
        {
            if (charMoveComp->MovementMode != MOVE_Walking && charMoveComp->MovementMode != MOVE_NavWalking)
            {
                charMoveComp->SetMovementMode(MOVE_Walking);
            }
        }
    }
}