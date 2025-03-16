// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAnimNotify_JumpStart.h"
#include "SoftDesignTraining.h"
#include "SDTAIController.h"
#include "SoftDesignTrainingMainCharacter.h"

void USDTAnimNotify_JumpStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    ASoftDesignTrainingMainCharacter* Character = Cast<ASoftDesignTrainingMainCharacter>(MeshComp->GetOwner());
    if (Character)
    {
        Character->BIsJumping = true;
        UE_LOG(LogTemp, Warning, TEXT("Jump Start Triggered: isJumping = true"));
    }
}

