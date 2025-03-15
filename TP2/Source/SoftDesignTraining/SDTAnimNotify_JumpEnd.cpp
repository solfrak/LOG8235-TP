// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAnimNotify_JumpEnd.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingCharacter.h"
#include "SoftDesignTrainingMainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void USDTAnimNotify_JumpEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    ASoftDesignTrainingMainCharacter* Character = Cast<ASoftDesignTrainingMainCharacter>(MeshComp->GetOwner());
    if (Character)
    {
        Character->isJumping = false;
        UE_LOG(LogTemp, Warning, TEXT("Jump End Triggered: isJumping = false"));
    }
}

