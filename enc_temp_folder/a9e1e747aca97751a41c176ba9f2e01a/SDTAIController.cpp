// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"


void ASDTAIController::Tick(float deltaTime)
{
	//TODO: calculate speed with accel and max_speed
	MoveToTarget(FVector2D(0, 0), 100, deltaTime);
	if (DetectWall())
	{
		printf("Hello");
	}

}

bool ASDTAIController::MoveToTarget(FVector2D target, float speed, float deltaTime)
{
	APawn* pawn = GetPawn();
	FVector const pawnPosition(pawn->GetActorLocation());
	FVector2D const toTarget = target - FVector2D(pawnPosition);
	FVector2D const displacement = FMath::Min(toTarget.Size(), speed * deltaTime) * toTarget.GetSafeNormal();
	pawn->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);
	pawn->SetActorRotation(FVector(displacement, 0.f).ToOrientationQuat());
	return toTarget.Size() < speed * deltaTime;
}

bool ASDTAIController::DetectWall()
{
	PhysicsHelpers helper(GetWorld());

	APawn* pawn = GetPawn();
	FVector const pawnPosition(pawn->GetActorLocation());

	TArray<FOverlapResult> results;
	helper.SphereOverlap(pawnPosition + pawn->GetActorForwardVector() * 100, 100, results, true);
	for (auto hit : results)
	{

	}

	return results.Num() != 0;
}






