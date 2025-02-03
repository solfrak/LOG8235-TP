// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"
#include "SDTUtils.h"
#include "Engine/StaticMeshActor.h"



void ASDTAIController::Tick(float deltaTime)
{
	m_wall_detected = TrueDetectWall(m_wall_detection_distance);
	if (m_wall_detected) {
		AvoidObstacle(deltaTime);
		//GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Blue, TEXT("hit wall"));
		//UE_LOG(LogTemp, Warning ,TEXT("testing is this works"));

		//call deplacement ici

	}
	//deplacement agent
	else{
		m_current_transition_duration = 0.0f;
		MoveToTarget(FVector2D(GetPawn()->GetActorLocation() + GetPawn()->GetActorForwardVector() * m_wall_detection_distance), m_max_speed, deltaTime);
	}
	//if (DetectWall())
	//{
		//GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Blue, TEXT("hit wall"));
	//}

}

bool ASDTAIController::MoveToTarget(FVector2D target, float speed, float deltaTime)
{
	APawn* pawn = GetPawn();
	FVector const pawnPosition(pawn->GetActorLocation());
	FVector2D const toTarget = target - FVector2D(pawnPosition);
	//calcul de la vitesse ici
	FVector2D const displacement = FMath::Min(toTarget.Size(), speed * deltaTime) * toTarget.GetSafeNormal();
	pawn->SetActorLocation(pawnPosition + FVector(displacement, 0.f), true);
	//orientation
	pawn->SetActorRotation(FVector(displacement, 0.f).ToOrientationQuat());
	return toTarget.Size() < speed * deltaTime;
}

bool ASDTAIController::DetectWall()
{
	PhysicsHelpers helper(GetWorld());

	APawn* pawn = GetPawn();
	FVector const pawnPosition(pawn->GetActorLocation());

	TArray<FOverlapResult> results;
	helper.SphereOverlap(pawnPosition + pawn->GetActorForwardVector() * 100, m_radius_detection, results, COLLISION_DEATH_OBJECT, true);
	

	for (const FOverlapResult& overlapResult : results)
	{
		if (overlapResult.GetActor()->ActorHasTag(FName("Death")))
		{
			printf("Hello");
		}

	}
	return results.Num() != 0;
}


bool ASDTAIController::TrueDetectWall(float distance) {

	//je raycast ici mais overlap devait etre faisable aussi je pense
	struct FHitResult hit;
	PhysicsHelpers help(GetWorld());
	APawn* pawn = GetPawn();
	//pawn->GetActorRightVector();
	FVector const pawnPosition(pawn->GetActorLocation());
	bool obstacleHit = help.CastSingleRay(pawnPosition, pawnPosition + pawn->GetActorForwardVector() * distance, hit, true);

	FString test = hit.ToString();

	if (obstacleHit) {
		AStaticMeshActor* wall = Cast<AStaticMeshActor>(hit.GetActor());
		if (wall != nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("WE HIT A WALL LETS GO? %s"), *hit.GetActor()->GetFName().ToString());
			//premiere idee cest de lerp entre forward vector et right vector
			//rotation du personnage ensuite jusqu'a ce quil ny ait plus de mur
		}

	}
	//le cast devrait etre retourner ici.
	return hit.GetActor() != nullptr;
}

void ASDTAIController::AvoidObstacle(float deltaTime) {
	m_current_transition_duration += deltaTime;
	float ratio = deltaTime / m_transition_duration;
	
	APawn* pawn = GetPawn();
	FVector const pawnPosition(pawn->GetActorLocation());
	FVector const forward(pawn->GetActorForwardVector());
	FVector const side(pawn->GetActorRightVector()); //jutilise side ici (basically un 90 degre), mais no idea sil faut un certain angle selon l'enonce du tp

	FVector const newDir = FMath::Lerp(forward, side, ratio * m_rotation_speed);
	FVector newLocation = pawnPosition + FVector(newDir).GetSafeNormal() * m_max_speed * deltaTime;
	pawn->SetActorLocation(newLocation, true);
	pawn->SetActorRotation((newDir).ToOrientationQuat());
	//pawn->SetActorRotation(FVector(pawnPosition + FVector(newDir) * m_max_speed), 0.0f).ToOrientationQuat();

}



