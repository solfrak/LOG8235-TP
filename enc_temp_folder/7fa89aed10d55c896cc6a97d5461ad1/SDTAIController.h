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
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;

    bool MoveToTarget(FVector2D target, float speed, float deltaTime);
    bool DetectWall();

private:
    UPROPERTY()
    float current_speed = 0.0f;

    UPROPERTY()
    float m_max_speed;

    UPROPERTY()
    float m_accel;

};
