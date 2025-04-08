// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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


	void RegisterAgent(ASDTAIController* aiAgent);
	void UnregisterAgent(ASDTAIController* aiAgent);

	UPROPERTY(EditAnywhere, Category = "Debug")
	FColor DebugBallColor = FColor::Orange;

	UPROPERTY(EditAnywhere, Category = "Debug")
	float DebugBallRadius = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bEnableDebugVisualization = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float deltaTime) override;

	void DrawDebugBallGroup();


private:	
	//singleton instance
	static ASDTAIManager* m_instance;

	//agents list that are chasing player
	UPROPERTY()
	TArray<ASDTAIController*> m_registeredAgents;

};
