// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTAIController.h"
#include "SDTAIManager.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API ASDTAIManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASDTAIManager();

	virtual void BeginDestroy() override;
	//singleton
	static ASDTAIManager* GetInstance();



	void UpdateAgentBestPosition();
	void RegisterAgent(ASDTAIController* aiAgent);
	void AssignPositionToAgent(ASDTAIController* agent);
	void UnregisterAgent(ASDTAIController* aiAgent);
	void CheckPursuitGroupDisbandCondition();

	void UpdateLKP();

	UPROPERTY(EditAnywhere, Category = "Debug")
	FColor DebugBallColor = FColor::Orange;

	UPROPERTY(EditAnywhere, Category = "Debug")
	float DebugBallRadius = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bEnableDebugVisualization = true;

	UPROPERTY(EditAnywhere, Category = "Debug")
	float sphere_cast_radius = 1000.0f;


	FVector player_LKP = FVector::Zero();

	void DisbandGroup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float deltaTime) override;

	void DrawDebugBallGroup();
	void DrawDebugClosestInterestPoint();


private:	
	//singleton instance
	static ASDTAIManager* m_instance;

	//agents list that are chasing player
	UPROPERTY()
	TSet<ASDTAIController*> m_registeredAgents;
	int round_robbin_assignation = 0;
	TArray<AActor*> m_closestInterestPoints;
	TSet<AActor*> m_assignedPOIsThisFrame;
};
