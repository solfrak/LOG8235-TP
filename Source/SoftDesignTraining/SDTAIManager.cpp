// Fill out your copyright notice in the Description page of Project Settings.


#include "SDTAIManager.h"
#include "DrawDebugHelpers.h"
#include "SoftDesignTraining.h"

//init static instance
ASDTAIManager* ASDTAIManager::m_instance = nullptr;

// Sets default values
ASDTAIManager::ASDTAIManager()
{
 	//actor call Tick() every frame. and set singleton instance
	PrimaryActorTick.bCanEverTick = true;
	m_instance = this;
}

void ASDTAIManager::BeginDestroy() 
{
	if (m_instance == this)
	{
		m_instance = nullptr;
	}
	Super::BeginDestroy();
}

// Called when the game starts or when spawned
void ASDTAIManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASDTAIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bEnableDebugVisualization)
		DrawDebugBallGroup();
}

ASDTAIManager* ASDTAIManager::GetInstance() 
{
	if (!m_instance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIManager instance not found. Make sure to place one in the level."));
	}
	return m_instance;
}

void ASDTAIManager::RegisterInterestPoint(AActor* point)
{
	m_interestPoints.Add(point);
}

void ASDTAIManager::UpdateAgentBestPosition()
{
	//TODO:logic pour encercler le joueurs
}

void ASDTAIManager::RegisterAgent(ASDTAIController* aIAgent)
{
	if (!aIAgent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to register null AI agent to pursuit group"));
		return;
	}

	if (!m_registeredAgents.Contains(aIAgent)) 
	{
		m_registeredAgents.Add(aIAgent);
	}
}

void ASDTAIManager::UnregisterAgent(ASDTAIController* aIAgent)
{
	if (!aIAgent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Attempted to unregister null AI agent from pursuit group"));
		return;
	}

	m_registeredAgents.Remove(aIAgent);
	
}

void ASDTAIManager::UpdateLKP()
{
	auto playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	auto location = playerCharacter->GetActorLocation();

	player_LKP = playerCharacter->GetActorLocation();
}


void ASDTAIManager::DrawDebugBallGroup()
{
	UWorld* world = GetWorld();
	if (!world)
		return;

	for (auto agent : m_registeredAgents)
	{
		if (!agent || !agent->GetPawn())
			continue;

		APawn* pawn = agent->GetPawn();

		FVector headPosition;
		FRotator rotation;
		agent->GetPawn()->GetActorEyesViewPoint(headPosition, rotation);

		DrawDebugSphere(world, headPosition, DebugBallRadius, 8, DebugBallColor);
	}


	DrawDebugSphere(world, player_LKP, DebugBallRadius, 8, FColor::Cyan);
}
