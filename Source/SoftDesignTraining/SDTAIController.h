// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PhysicsHelpers.h"

#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual ~ASDTAIController();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

protected:
	void UpdateMovement(float DeltaTime);

	void DetectAndAvoidObstacles(float DeltaTime, APawn* ControlledPawn);

	void DetectAndCollectPickup(float DeltaTime, APawn* ControlledPawn);

	void ChaseCollectible(const FVector& PickupLocation);

	void MoveForward(float DeltaTime);

	void AdjustVelocity(float value);

	void RotateAwayFromWall(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Acceleration = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeed = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RotationSpeed = 90.0f;

	FVector Velocity = FVector::ZeroVector;

	bool bIsAvoidingWall = false;
	bool bIsAvoidingDeathFloor = false;
	bool bIsChasingPickup = false;

	int32 RandomTurnDirection = 0;
	float WallClearTimer = 0.0f;

	FVector CurrentPickupLocation = FVector::ZeroVector;

	class PhysicsHelpers* PhysicsHelper = nullptr;
};
