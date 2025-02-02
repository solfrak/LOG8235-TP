// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "PhysicsHelpers.h"
#include "SDTUtils.h"
#include "Engine/StaticMeshActor.h"

void ASDTAIController::Tick(float deltaTime)
{

	//deplacement agent
	MoveToTarget(FVector2D(0, 0), m_max_speed, deltaTime);
	if (CastRay(FVector2D(0, 0))) {
		//GEngine->AddOnScreenDebugMessage(-1, 2.5f, FColor::Blue, TEXT("hit wall"));
		//UE_LOG(LogTemp, Warning ,TEXT("testing is this works"));

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


bool ASDTAIController::CastRay(FVector2D targetPos) {



	//Multi line trace
	FCollisionObjectQueryParams objectQueryParams;
	objectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	objectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	queryParams.bReturnPhysicalMaterial = true;
	struct FHitResult hit;

	//m_world->LineTraceMultiByObjectType(outHits, start, end, objectQueryParams, queryParams);

	TArray<struct FHitResult> results;
	PhysicsHelpers help(GetWorld());
	APawn* pawn = GetPawn();
	FVector const pawnPosition(pawn->GetActorLocation());
	//help.CastRay(pawnPosition, FVector(targetPos, pawnPosition.Z), results, true);
	DrawDebugLine(GetWorld(), pawnPosition, FVector(targetPos, pawnPosition.Z), FColor::Green);
	GetWorld()->LineTraceSingleByObjectType(hit, pawnPosition, FVector(targetPos, pawnPosition.Z), objectQueryParams, queryParams);

	FString test = hit.ToString();
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *test);
	//if (hit.GetActor() != nullptr) {
	//	UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *hit.GetActor()->GetClass()->GetName());
	//}
	if (hit.GetActor() != nullptr) {
		AStaticMeshActor* wall = Cast<AStaticMeshActor>(hit.GetActor());
		if (wall != nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("WE HIT A WALL LETS GO? %s"), *hit.GetActor()->GetFName().ToString());
		}

	}

	//UE_LOG(LogTemp, Display, TEXT("the amount of hit is: %d"), results.Num());
	//for (const FHitResult& result : results) {
		//ca hit les walls ici, on veut le premier wall
		//FString test = result.ToString();
		//UE_LOG(LogTemp, Warning, TEXT("% s"), *test);
		//result.Component.Get()->AddForce(FVector(1000000.0f, 1000000.0f, 1000000.0f));
		//AActor* testing = result.GetActor();

	//}
	return results.Num() > 0;
}





