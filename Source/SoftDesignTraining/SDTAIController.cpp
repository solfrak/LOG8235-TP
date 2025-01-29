// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"

void ASDTAIController::Tick(float deltaTime)
{
	MoveToTarget(FVector2D(0,0), m_maxSpeed, deltaTime);
}

bool ASDTAIController::MoveToTarget(FVector2D target, float speed, float deltaTime){
	APawn* pawn = GetPawn();
	FVector const pawnPosition(pawn->GetActorLocation());
	FVector2D const toTarget = target - FVector2D(pawnPosition);
	FVector2D const displacement = FMath::Min(toTarget.Size(), speed * deltaTime) * toTarget.GetSafeNormal();
	pawn->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);
	pawn->SetActorRotation(FVector(displacement, 0).ToOrientationQuat());
	return toTarget.Size() < speed * deltaTime;
}

bool ASDTAIController::IsInsideCone(APawn* pawn, AWallActor* targetActor) const
{
    if (FVector::Dist2D(pawn->GetActorLocation(), targetActor->GetActorLocation()) > 500.f)
    {
        return false;
    }

    auto pawnForwardVector = pawn->GetActorForwardVector();
    auto dir = targetActor->GetActorLocation() - pawn->GetActorLocation();

    auto value = FVector::DotProduct(dir.GetSafeNormal(), pawnForwardVector.GetSafeNormal());
    auto angle = FMath::Acos(value);

    auto isVisible = FMath::Abs(angle) <= m_visionAngle;

    if (isVisible)
    {
        DrawDebugSphere(GetWorld(), targetActor->GetActorLocation(), 100.f, 32, FColor::Magenta);
    }

    GEngine->AddOnScreenDebugMessage(-1, 0.16f, FColor::Red, FString::Printf(TEXT("Actor : %s is %s"), *targetActor->GetName(), isVisible ? TEXT("visible") : TEXT("not visible")));

    return isVisible;
}








