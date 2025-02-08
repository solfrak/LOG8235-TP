#include "SDTCollectible.h"
#include "SoftDesignTraining.h"
#include "Kismet/GameplayStatics.h"

ASDTCollectible::ASDTCollectible()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ASDTCollectible::BeginPlay()
{
    Super::BeginPlay();
}

void ASDTCollectible::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASDTCollectible::Collect()
{
    // Démarrer le cooldown
    GetWorld()->GetTimerManager().SetTimer(m_CollectCooldownTimer, this, &ASDTCollectible::OnCooldownDone, m_CollectCooldownDuration, false);

    // Déclencher le son de feedback s'il est assigné
    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }

    // Déclencher l'effet FX s'il est assigné
    if (PickupFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupFX, GetActorLocation(), FRotator::ZeroRotator, true);
    }

    // Masquer le pickup
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
