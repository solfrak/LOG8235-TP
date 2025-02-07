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

bool ASDTAIController::DetectTarget(FVector& OutTargetLocation, AActor*& OutTargetActor, bool& bIsCollectible)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
		return false;

	FVector PawnLocation = ControlledPawn->GetActorLocation();
	float DetectionRadius = 500.0f;
	TArray<FOverlapResult> OverlapResults;
	bool bFound = PhysicsHelper->SphereOverlap(PawnLocation, DetectionRadius, OverlapResults, true);

	if (bFound)
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			if (Overlap.GetComponent())
			{
				ECollisionChannel Channel = Overlap.GetComponent()->GetCollisionObjectType();
				// Détection du collectible
				if (Channel == ECC_GameTraceChannel5)
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
				// Détection du joueur
				else if (Overlap.GetActor()->IsA(ASoftDesignTrainingMainCharacter::StaticClass()))
				{
					ASoftDesignTrainingMainCharacter* MainChar = Cast<ASoftDesignTrainingMainCharacter>(Overlap.GetActor());
					if (MainChar && !MainChar->IsPoweredUp())
					{
						OutTargetLocation = MainChar->GetActorLocation();
						OutTargetActor = MainChar;
						bIsCollectible = false;
						return true;
					}
				}
			}
		}
	}
	return false;
}

void ASDTAIController::UpdateMovement(float DeltaTime)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !PhysicsHelper)
		return;

	FVector PawnLocation = ControlledPawn->GetActorLocation();
	FVector Forward = ControlledPawn->GetActorForwardVector();

	// Recast en face de l'agent pour détecter un mur
	TArray<FHitResult> Hits;
	bool bWallDetected = PhysicsHelper->CastRay(PawnLocation, PawnLocation + Forward * 100.0f, Hits, true);

	// Raycast pour detecter un death trap
	FVector RayStart = PawnLocation + Forward * 100.0f + FVector(0,0,-100.0f);
	bool bObstacleDetected = PhysicsHelper->SphereCast(RayStart, RayStart, 100.0f, Hits, true);

	bool bDeathTrapDetected = false;
	if (bObstacleDetected) {
		for (const FHitResult& Hit : Hits) {
			if (Hit.GetComponent() && Hit.GetComponent()->GetCollisionObjectType() == ECC_GameTraceChannel3) {
				bDeathTrapDetected = true;
				break;
			}
		}
	}

	// Recherche de cible avec sphere overlap
	FVector TargetLocation;
	AActor* TargetActor = nullptr;
	bool bIsCollectible = false;
	bool bTargetDetected = DetectTarget(TargetLocation, TargetActor, bIsCollectible);

	if (bTargetDetected)
	{
		bool bClear = IsPathClear(PawnLocation, TargetLocation, TargetActor);
		if (bClear)
		{
			if (!bIsCollectible)
			{
				ChasePlayer(TargetLocation);
			}
			else
			{
				ChaseCollectible(TargetLocation);
			}
			return;
		}
	}

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
	float AcceptanceRadius = 20.0f;
	MoveToLocation(PickupLocation, AcceptanceRadius, true, true, true, false);
	bIsChasingPickup = true;
}

void ASDTAIController::ChasePlayer(const FVector& PlayerLocation)
{
	float AcceptanceRadius = 20.0f;
	MoveToLocation(PlayerLocation, AcceptanceRadius, true, true, true, false);
	bIsPursuingPlayer = true;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	bIsChasingPickup = false;
	bIsPursuingPlayer = false;
}

void ASDTAIController::MoveForward(float DeltaTime)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
		return;

	FVector DesiredDirection = ControlledPawn->GetActorForwardVector();
	Velocity += DesiredDirection * Acceleration * DeltaTime;
	Velocity = Velocity.GetClampedToMaxSize(MaxSpeed);
	ControlledPawn->AddMovementInput(Velocity.GetSafeNormal(), Velocity.Size());
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
