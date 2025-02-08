// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTCollectible.h"
#include "SoftDesignTraining.h"

ASDTCollectible::ASDTCollectible()
{
    PrimaryActorTick.bCanEverTick = true;

    // Define the asset path as a variable.
    const FString SoundAssetPath = TEXT("/Game/assets/pickup_sound.pickup_sound");
    const FString VFXAssetPath = TEXT("/Game/assets/P_Explosion.P_Explosion");

    // Log the paths we're trying to load.
    UE_LOG(LogTemp, Warning, TEXT("Trying to load pickup sound asset from: %s"), *SoundAssetPath);
    UE_LOG(LogTemp, Warning, TEXT("Trying to load pickup VFX asset from: %s"), *VFXAssetPath);

    // Use FObjectFinder to load the sound asset.
    static ConstructorHelpers::FObjectFinder<USoundBase> SoundFinder(*SoundAssetPath);
    if (SoundFinder.Succeeded())
    {
        CollectionSound = SoundFinder.Object;
        UE_LOG(LogTemp, Warning, TEXT("SDTCollectible: Successfully loaded pickup sound asset."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SDTCollectible: Could not find the pickup sound asset."));
    }

    // Use FObjectFinder to load the VFX asset.
    static ConstructorHelpers::FObjectFinder<UParticleSystem> FXFinder(*VFXAssetPath);
    if (FXFinder.Succeeded())
    {
        CollectionFX = FXFinder.Object;
        UE_LOG(LogTemp, Warning, TEXT("SDTCollectible: Successfully loaded pickup VFX asset."));
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
