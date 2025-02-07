#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "SDTCollectible.h"

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

void ASDTAIController::UpdateMovement(float DeltaTime)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !PhysicsHelper) return;

	DetectAndAvoidObstacles(DeltaTime, ControlledPawn);

	if (!bIsAvoidingWall && !bIsAvoidingDeathFloor)
	{
		WallClearTimer += DeltaTime;
		if (WallClearTimer > 0.5f)
		{
			RandomTurnDirection = 0;
			WallClearTimer = 0.0f;
		}
		DetectAndCollectPickup(DeltaTime, ControlledPawn);
		if (!bIsChasingPickup)
		{
			MoveForward(DeltaTime);
		}
	}
	else
	{
		WallClearTimer = 0.0f;
		AdjustVelocity(0.5f);
	}
}

void ASDTAIController::DetectAndAvoidObstacles(float DeltaTime, APawn* ControlledPawn)
{
	FVector Start = ControlledPawn->GetActorLocation();
	FVector Forward = ControlledPawn->GetActorForwardVector() * 100.0f;
	FVector Down = ControlledPawn->GetActorUpVector() * -10.0f;
	FVector Position = Start + Forward + Down;
	float DetectionRadius = 100.0f;

	TArray<FOverlapResult> Overlaps;
	bool bObstacleDetected = PhysicsHelper->SphereOverlap(Position, DetectionRadius, Overlaps, true);

	bIsAvoidingWall = false;
	bIsAvoidingDeathFloor = false;

	if (bObstacleDetected)
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetComponent())
			{
				ECollisionChannel OverlapChannel = Overlap.GetComponent()->GetCollisionObjectType();
				if (OverlapChannel == ECC_GameTraceChannel3)
				{
					bIsAvoidingDeathFloor = true;
					AdjustVelocity(0.0f);
					RotateAwayFromWall(DeltaTime);
				}
				else if (OverlapChannel == ECC_WorldStatic)
				{
					bIsAvoidingWall = true;
					RotateAwayFromWall(DeltaTime);
				}
			}
		}
	}
}

void ASDTAIController::DetectAndCollectPickup(float DeltaTime, APawn* ControlledPawn)
{
	if (bIsChasingPickup)
		return;

	FVector PawnLocation = ControlledPawn->GetActorLocation();
	float DetectionRadius = 500.0f;
	TArray<FOverlapResult> Overlaps;
	bool bPickupDetected = PhysicsHelper->SphereOverlap(PawnLocation, DetectionRadius, Overlaps, true);

	if (bPickupDetected)
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetComponent())
			{
				ECollisionChannel OverlapChannel = Overlap.GetComponent()->GetCollisionObjectType();
				if (OverlapChannel == ECC_GameTraceChannel5)
				{
					ASDTCollectible* Collectible = Cast<ASDTCollectible>(Overlap.GetActor());
					if (Collectible)
					{
						if (!Collectible->IsOnCooldown())
						{
							bIsChasingPickup = true;
							CurrentPickupLocation = Collectible->GetActorLocation();
							ChaseCollectible(CurrentPickupLocation);
							return;
						}
						else
						{
							UE_LOG(LogTemp, Log, TEXT("Collectible détecté mais en cooldown."));
						}
					}
				}
			}
		}
	}
}

void ASDTAIController::ChaseCollectible(const FVector& PickupLocation)
{
	float AcceptanceRadius = 20.0f;
	EPathFollowingRequestResult::Type MoveResult = MoveToLocation(PickupLocation, AcceptanceRadius, true, true, true, false);
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	if (bIsChasingPickup)
	{
		bIsChasingPickup = false;
		UE_LOG(LogTemp, Log, TEXT("Collectible reached and collected."));
	}
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
	if (!ControlledPawn) return;

	if (RandomTurnDirection == 0)
	{
		RandomTurnDirection = FMath::RandBool() ? 1 : -1;
		UE_LOG(LogTemp, Log, TEXT("Direction choisie: %d"), RandomTurnDirection);
	}

	FRotator CurrentRotation = ControlledPawn->GetActorRotation();
	FRotator NewRotation = CurrentRotation + FRotator(0.0f, RandomTurnDirection * RotationSpeed * DeltaTime, 0.0f);
	ControlledPawn->SetActorRotation(NewRotation);
}
