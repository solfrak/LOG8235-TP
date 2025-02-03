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
    bool TrueDetectWall(float distance);
    void AvoidObstacle(float deltaTime);

private:
    UPROPERTY(EditAnywhere)
    float current_speed = 0.0f;

    UPROPERTY(EditAnywhere)
    float m_max_speed;

    UPROPERTY(EditAnywhere)
    float m_accel;

    UPROPERTY(EditAnywhere)
    float m_radius_detection = 100;

    UPROPERTY(EditAnywhere)
    float m_wall_detection_distance = 100.0f;

    UPROPERTY(EditAnywhere)
    float m_transition_duration = 3.0f;

    float m_current_transition_duration = 0.0f;

    bool m_wall_detected = false;

};
