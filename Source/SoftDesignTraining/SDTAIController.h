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

    //Macro pour intéragir avec blueprint du éditeur
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    float PickupDetectionRange = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
    float PickupCollectionThreshold = 50.f;

    UFUNCTION(BlueprintCallable, Category = "Pickup")
    bool DetectPickup(FVector& vector);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyMovement(FVector direction);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyRotation(FVector direction);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateVelocity(float deltaTime);

    //called each frame
    virtual void Tick(float deltaTime) override;

    bool MoveToTarget(FVector2D target, float speed, float deltaTime);
    bool DetectObstacle(FVector& normal);
    void AvoidObstacle(float deltaTime);

private:

    UPROPERTY(EditAnywhere)
    float current_speed = 0.0f;

    UPROPERTY(EditAnywhere)
    float m_max_speed = 600.0f;

    UPROPERTY(EditAnywhere)
    float m_accel = 200.0f;

    UPROPERTY(EditAnywhere)
    float m_radius_detection = 100;

    UPROPERTY(EditAnywhere)
    float m_wall_detection_distance = 100.0f;

    UPROPERTY(EditAnywhere)
    float m_transition_duration = 3.0f;

    UPROPERTY(EditAnywhere)
    float m_rotation_speed = 15.0f;

    UPROPERTY(EditAnywhere)
    float m_pickup_detection_range = 300.0f;

    UPROPERTY(EditAnywhere)
    float m_pickup_detection_radius = 300.0f;

    float m_current_transition_duration = 0.0f;

    bool m_wall_detected = false;

};
