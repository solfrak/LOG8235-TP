#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "SDTCollectible.h"
#include "SoftDesignTrainingMainCharacter.h"

ASDTAIController::~ASDTAIController()
{
    delete PhysicsHelper;
}

void ASDTAIController::BeginPlay()
{
    Super::BeginPlay();
    PhysicsHelper = new PhysicsHelpers(GetWorld());
}

void ASDTAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateMovement(DeltaTime);
}

// Fonction qui vérifie si la ligne de visée entre Start et End est dégagée (en utilisant le CastRay fourni)
bool ASDTAIController::IsPathClear(const FVector& Start, const FVector& End, AActor* TargetActor)
{
    TArray<FHitResult> Hits;
    bool bHit = PhysicsHelper->CastRay(Start, End, Hits, false);
    if (!bHit)
    {
        return true;
    }
    for (const FHitResult& Hit : Hits)
    {
        if (Hit.GetActor() != nullptr && Hit.GetActor() != TargetActor)
        {
            return false;
        }
    }
    return true;
}

// Fonction qui effectue un sphere overlap autour de l'agent pour détecter une cible.
// Ici, on parcourt d'abord les overlaps pour rechercher un joueur ; s'il est trouvé, on le retourne.
// Sinon, on vérifie la présence d'un collectible.
bool ASDTAIController::DetectTarget(FVector& OutTargetLocation, AActor*& OutTargetActor, bool& bIsCollectible)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return false;

    FVector PawnLocation = ControlledPawn->GetActorLocation();
    float DetectionRadius = 500.0f;
    TArray<FOverlapResult> OverlapResults;
    bool bFound = PhysicsHelper->SphereOverlap(PawnLocation, DetectionRadius, OverlapResults, true);

    // Priorité au joueur
    if (bFound)
    {
        for (const FOverlapResult& Overlap : OverlapResults)
        {
            if (Overlap.GetComponent() && Overlap.GetActor())
            {
                ASoftDesignTrainingMainCharacter* PlayerChar = Cast<ASoftDesignTrainingMainCharacter>(Overlap.GetActor());
                if (PlayerChar)
                {
                    OutTargetLocation = PlayerChar->GetActorLocation();
                    OutTargetActor = PlayerChar;
                    bIsCollectible = false;
                    return true;
                }
            }
        }
    }
    // Sinon, rechercher un collectible
    if (bFound)
    {
        for (const FOverlapResult& Overlap : OverlapResults)
        {
            if (Overlap.GetComponent() && Overlap.GetActor())
            {
                ASDTCollectible* Collectible = Cast<ASDTCollectible>(Overlap.GetActor());
                if (Collectible && !Collectible->IsOnCooldown())
                {
                    OutTargetLocation = Collectible->GetActorLocation();
                    OutTargetActor = Collectible;
                    bIsCollectible = true;
                    return true;
                }
            }
        }
    }
    return false;
}

// Comportement de fuite : oriente l'agent dans la direction opposée au joueur et avance.
void ASDTAIController::FleePlayerFrom(const FVector& PlayerLocation, float DeltaTime)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    FVector AgentLocation = ControlledPawn->GetActorLocation();
    FVector FleeDirection = (AgentLocation - PlayerLocation).GetSafeNormal();
    FRotator TargetRotation = FleeDirection.Rotation();
    FRotator NewRotation = FMath::RInterpTo(ControlledPawn->GetActorRotation(), TargetRotation, DeltaTime, 5.0f);
    ControlledPawn->SetActorRotation(NewRotation);

    MoveForward(DeltaTime);
    bIsFleeingPlayer = true;
}

void ASDTAIController::UpdateMovement(float DeltaTime)
{
    UpdateVelocity(DeltaTime);
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn || !PhysicsHelper)
        return;
    ControlledPawn->GetComponentByClass<UCharacterMovementComponent>()->MaxWalkSpeed = Velocity.Size();

    FVector PawnLocation = ControlledPawn->GetActorLocation();
    FVector Forward = ControlledPawn->GetActorForwardVector();

    // 1. Recast pour détecter un mur en face de l'agent (utilisation du CastRay fourni)
    TArray<FHitResult> WallHits;
    bool bWallDetected = PhysicsHelper->CastRay(PawnLocation, PawnLocation + Forward * 100.0f, WallHits, true);

    // 2. Raycast pour détecter un death trap (via SphereCast)
    FVector RayStart = PawnLocation + Forward * 100.0f + FVector(0, 0, -100.0f);
    TArray<FHitResult> DeathTrapHits;
    bool bObstacleDetected = PhysicsHelper->SphereCast(RayStart, RayStart, 100.0f, DeathTrapHits, true);
    bool bDeathTrapDetected = false;
    if (bObstacleDetected)
    {
        for (const FHitResult& Hit : DeathTrapHits)
        {
            if (Hit.GetComponent() && Hit.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel3)
            {
                bDeathTrapDetected = true;
                break;
            }
        }
    }

    // 3. Détection de cible via sphere overlap (priorité au joueur)
    FVector TargetLocation;
    AActor* TargetActor = nullptr;
    bool bIsCollectible = false;
    bool bTargetDetected = DetectTarget(TargetLocation, TargetActor, bIsCollectible);

    if (bTargetDetected)
    {
        // Si la cible est le joueur
        if (!bIsCollectible)
        {
            ASoftDesignTrainingMainCharacter* MainChar = Cast<ASoftDesignTrainingMainCharacter>(TargetActor);
            if (MainChar)
            {
                // Si le joueur est power-up, l'agent doit fuir (ignorer les pickups)
                if (MainChar->IsPoweredUp())
                {
                    FleePlayerFrom(TargetLocation, DeltaTime);
                    return;
                }
                else
                {
                    bool bClear = IsPathClear(PawnLocation, TargetLocation, MainChar);
                    if (bClear)
                    {
                        ChasePlayer(TargetLocation);
                        return;
                    }
                }
            }
        }
        // Sinon, si la cible est un pickup
        else
        {
            bool bClear = IsPathClear(PawnLocation, TargetLocation, TargetActor);
            if (bClear)
            {
                ChaseCollectible(TargetLocation);
                return;
            }
        }
    }

    // 4. Si un mur ou un death trap est détecté, l'agent effectue l'évitement
    if (bWallDetected || bDeathTrapDetected)
    {
        AdjustVelocity(0.5f);
        RotateAwayFromWall(DeltaTime);
    }
    else
    {
        MoveForward(DeltaTime);
    }
}

void ASDTAIController::ChaseCollectible(const FVector& PickupLocation)
{
    //this uses the walkspeed
    float AcceptanceRadius = 20.0f;
    MoveToLocation(PickupLocation, AcceptanceRadius, true, true, true, false);
    bIsChasingPickup = true;
}

void ASDTAIController::ChasePlayer(const FVector& PlayerLocation)
{
    //this uses the walkspeed
    float AcceptanceRadius = 20.0f;
    MoveToLocation(PlayerLocation, AcceptanceRadius, true, true, true, false);
    bIsPursuingPlayer = true;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
    Super::OnMoveCompleted(RequestID, Result);
    bIsChasingPickup = false;
    bIsPursuingPlayer = false;
    bIsFleeingPlayer = false;
}

void ASDTAIController::MoveForward(float DeltaTime)
{
    //this uses our velocity
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    FVector ForwardVector = ControlledPawn->GetActorForwardVector();
    ControlledPawn->AddMovementInput(ForwardVector, Velocity.Size());
}

void ASDTAIController::AdjustVelocity(float value)
{
    Velocity *= value;
}

void ASDTAIController::RotateAwayFromWall(float DeltaTime)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    if (RandomTurnDirection == 0)
    {
        RandomTurnDirection = FMath::RandBool() ? 1 : -1;
        UE_LOG(LogTemp, Log, TEXT("Direction choisie: %d"), RandomTurnDirection);
    }

    FRotator CurrentRotation = ControlledPawn->GetActorRotation();
    FRotator NewRotation = CurrentRotation + FRotator(0.0f, RandomTurnDirection * RotationSpeed * DeltaTime, 0.0f);
    ControlledPawn->SetActorRotation(NewRotation);
}

void ASDTAIController::UpdateVelocity(float deltaTime) {
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
        return;

    FVector DesiredDirection = ControlledPawn->GetActorForwardVector();
    Velocity += DesiredDirection * Acceleration * deltaTime;
    Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
}