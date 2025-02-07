// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTCollectible.h"
#include "SoftDesignTraining.h"

ASDTCollectible::ASDTCollectible()
{
    PrimaryActorTick.bCanEverTick = true;

    //SFX
    static ConstructorHelpers::FObjectFinder<USoundBase> SoundFinder(TEXT("/Game/assets/pickup_sound.pickup_sound"));
    if (SoundFinder.Succeeded())
    {
        CollectionSound = SoundFinder.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SDTCollectible: Could not find the pickup sound asset."));
    }

    //VFX
    static ConstructorHelpers::FObjectFinder<UParticleSystem> FXFinder(TEXT("/Game/assets/P_Explosion.P_Explosion"));
    if (FXFinder.Succeeded())
    {
        CollectionFX = FXFinder.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SDTCollectible: Could not find the pickup VFX asset."));
    }
}

void ASDTCollectible::BeginPlay()
{
    Super::BeginPlay();
}

void ASDTCollectible::Collect()
{

    //SFX
    if (CollectionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CollectionSound, GetActorLocation());
    }

    //FX
    if (CollectionFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CollectionFX, GetActorLocation());
    }

    GetWorld()->GetTimerManager().SetTimer(m_CollectCooldownTimer, this, &ASDTCollectible::OnCooldownDone, m_CollectCooldownDuration, false);

    GetStaticMeshComponent()->SetVisibility(false);
}

void ASDTCollectible::OnCooldownDone()
{
    GetWorld()->GetTimerManager().ClearTimer(m_CollectCooldownTimer);

    GetStaticMeshComponent()->SetVisibility(true);
}

bool ASDTCollectible::IsOnCooldown()
{
    return m_CollectCooldownTimer.IsValid();
}

void ASDTCollectible::Tick(float deltaTime)
{
    Super::Tick(deltaTime);
}
