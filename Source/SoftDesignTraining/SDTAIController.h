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

private:
    virtual bool MoveToTarget(FVector2D target, float speed, float deltaTime);
    virtual void GetVisibilityInformationForTarget(AWallActor* targetActor, FVector selfPosition, PhysicsHelpers& physicHelper, bool& canSee, bool& canShoot);
    virtual void DrawCharacterAxes(UWorld* world, APawn* pawn) const;
    virtual void DrawVisionCone(UWorld* world, APawn* pawn, FVector const& dir, float angle) const;
    virtual TArray<FOverlapResult> CollectTargetActorsInFrontOfCharacter(APawn const* pawn, PhysicsHelpers& physicHelper) const;
    virtual bool IsInsideCone(APawn* pawn, AWallActor* targetActor) const;
public:
    virtual void Tick(float deltaTime) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Movement")
    float m_maxSpeed = 500.0f;

    float const m_maxVisionAngle = PI / 3.0f;
};
