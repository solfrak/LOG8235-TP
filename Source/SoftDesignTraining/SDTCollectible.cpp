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
    // D�marrer le cooldown

    // D�clencher le son de feedback s'il est assign�
    if (PickupSound && !GetWorld()->GetTimerManager().IsTimerActive(m_CollectCooldownTimer))
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }

    // D�clencher l'effet FX s'il est assign�
    if (PickupFX && !GetWorld()->GetTimerManager().IsTimerActive(m_CollectCooldownTimer))
    {
        UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PickupFX, GetActorLocation(), FRotator::ZeroRotator, true);
        if (ParticleComp)
        {
            FTimerHandle TimerHandle;
            //lambda pour detruire les particules apres un certain temps
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [ParticleComp]() {
                ParticleComp->DestroyComponent();
                }, 1.0f, false);
        }

        // Masquer le pickup
        GetStaticMeshComponent()->SetVisibility(false);
        GetWorld()->GetTimerManager().SetTimer(m_CollectCooldownTimer, this, &ASDTCollectible::OnCooldownDone, m_CollectCooldownDuration, false);
    }
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
