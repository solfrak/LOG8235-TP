// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()    //generates additional code for unreal
public:
    void DoMovement(float deltaTime, APawn* pawn);
	void Tick(float dt) override;

private:
	TArray<FHitResult> GetObstacleInPath();
	void MoveTowardCollectable();

protected:
	FVector m_velocity = FVector::ForwardVector;
	float m_max_speed = 100;
	float m_acceleration = 1;
};
