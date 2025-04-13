// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SoftDesignTrainingCharacter.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "SDTAIController.h"
#include "SDTProjectile.h"
#include "SDTUtils.h"
#include "DrawDebugHelpers.h"
#include "SDTCollectible.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h" // Include for AController
#include "SDTAIManager.h"
#include "SoftDesignTrainingPlayerController.h"


ASoftDesignTrainingCharacter::ASoftDesignTrainingCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
}

void ASoftDesignTrainingCharacter::BeginPlay()
{
    Super::BeginPlay();

    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASoftDesignTrainingCharacter::OnBeginOverlap);
    m_StartingPosition = GetActorLocation();
}

void ASoftDesignTrainingCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherComponent->GetCollisionObjectType() == COLLISION_DEATH_OBJECT)
    {
        Die();
    }
    else if(ASDTCollectible* collectibleActor = Cast<ASDTCollectible>(OtherActor))
    {
        if (!collectibleActor->IsOnCooldown())
        {
            OnCollectPowerUp();
        }

        collectibleActor->Collect();
    }
    else if (ASoftDesignTrainingMainCharacter* mainCharacter = Cast<ASoftDesignTrainingMainCharacter>(OtherActor))
    {
        if (mainCharacter->IsPoweredUp())
            Die();
    }
}

//void ASoftDesignTrainingCharacter::Die()
//{
//    SetActorLocation(m_StartingPosition);
//
//    if (ASDTAIController* controller = Cast<ASDTAIController>(GetController()))
//    {
//        controller->AIStateInterrupted();
//    }
//}

void ASoftDesignTrainingCharacter::Die()
{
    // Get Controller first
    AController* CurrentController = GetController();

    // Get Movement Component using the correct ACharacter function
    UCharacterMovementComponent* MoveComp = GetCharacterMovement(); // Correct function name

    // Teleport to Start
    SetActorLocation(m_StartingPosition);

    // Reset State AFTER Teleporting
    if (MoveComp) // Always check if the pointer is valid
    {
        MoveComp->StopMovementImmediately();
        MoveComp->SetMovementMode(MOVE_Walking); // Force back to walking
        MoveComp->Velocity = FVector::ZeroVector; // Reset velocity
    }
    else
    {
        UE_LOG(LogSoftDesignTraining, Error, TEXT("Character %s could not get CharacterMovementComponent in Die()!"), *GetName());
    }

    if (ASoftDesignTrainingPlayerController* mainCharacter = Cast<ASoftDesignTrainingPlayerController>(GetController())) {
        ASDTAIManager::GetInstance()->DisbandGroup();
    }

    if (ASDTAIController* controller = Cast<ASDTAIController>(CurrentController))
    {
        // Reset controller flags related to jumping/movement state
        controller->InAir = false;
        controller->Landing = false;
        controller->AtJumpSegment = false;

        // Call AIStateInterrupted AFTER resetting movement state
        controller->AIStateInterrupted();
    }
    else
    {
        if (CurrentController)
        {
            UE_LOG(LogSoftDesignTraining, Warning, TEXT("Character %s died but Controller is not an ASDTAIController."), *GetName());
        }
        else
        {
            UE_LOG(LogSoftDesignTraining, Warning, TEXT("Character %s died but has no Controller."), *GetName());
        }
    }
}