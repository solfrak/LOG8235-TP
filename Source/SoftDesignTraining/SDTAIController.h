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
    ASDTAIController();

    //Macro pour intéragir avec blueprint du éditeur
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetMovementDirection() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyMovement();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ApplyRotation();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateVelocity(float deltaTime);

    //called each frame
    virtual void Tick(float deltaTime) override;
};
