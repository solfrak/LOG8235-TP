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
	
	UpdateAgentBestPosition();
	if (bEnableDebugVisualization)
	{
		DrawDebugBallGroup();
		DrawDebugClosestInterestPoint();

	}

	CheckPursuitGroupDisbandCondition();
}

void ASDTAIManager::CheckPursuitGroupDisbandCondition()
{
	bool bAllAgentsLostSight = true;
	for (ASDTAIController* agent : m_registeredAgents)
	{
		if (agent && agent->TimeSinceLastSeenPlayer < 3.0f)
		{
			bAllAgentsLostSight = false;
			break;
		}
	}

	if (bAllAgentsLostSight)
	{
		UE_LOG(LogTemp, Log, TEXT("Dissolution du groupe de poursuite: tous les agents n'ont pas vu le joueur depuis au moins 3 secondes."));
		TSet<ASDTAIController*> AgentsCopy = m_registeredAgents;
		for (ASDTAIController* agent : AgentsCopy)
		{
			if (agent)
			{
				agent->LeavePursuitGroup();
			}
		}
	}
}


ASDTAIManager* ASDTAIManager::GetInstance() 
{
	if (!m_instance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIManager instance not found. Make sure to place one in the level."));
	}
	return m_instance;
}

void ASDTAIManager::UpdateAgentBestPosition()
{
	FCollisionQueryParams Params;
	m_closestInterestPoints.Empty();
	TArray<FOverlapResult> results;
	bool bOverlap = GetWorld()->OverlapMultiByObjectType(
		results,
		player_LKP,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_GameTraceChannel2), // If your channel is at index 1
		FCollisionShape::MakeSphere(sphere_cast_radius),
		Params
	);

	for (auto result : results)
	{
		m_closestInterestPoints.Add(result.GetActor());
	}

}

void ASDTAIManager::AssignPositionToAgent(ASDTAIController* agent)
{
	if (!m_registeredAgents.Contains(agent))
		return;

	if (!agent->is_following_player && m_closestInterestPoints.Num() > 0 && !m_closestInterestPoints.Contains(agent->current_interest_point))
	{
		UE_LOG(LogTemp, Warning, TEXT("Agent go to random intereste point"));
		int index = round_robbin_assignation++ % m_closestInterestPoints.Num();
		agent->current_interest_point = m_closestInterestPoints[index];
		agent->m_TargetPosition = agent->current_interest_point->GetActorLocation();
	}

	if (agent->current_interest_point != nullptr)
	{
		float dist_to_interest_point = FVector::Dist(agent->GetPawn()->GetActorLocation(), agent->current_interest_point->GetActorLocation());
		float threshold = 100.f;

		float dist_to_LKP = FVector::Dist(agent->GetPawn()->GetActorLocation(), player_LKP);
		float threshold2 = 500.f;

		if (dist_to_interest_point < threshold && dist_to_LKP < threshold2)
		{
			agent->is_following_player = true;
		}
		else {
			agent->is_following_player = false;
		}
	}

	if (agent->is_following_player)
	{

		agent->m_TargetPosition = player_LKP;
	}
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
	aIAgent->is_following_player = false;
	
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

void ASDTAIManager::DrawDebugClosestInterestPoint()
{
		DrawDebugSphere(GetWorld(), player_LKP, sphere_cast_radius, 8, FColor::Green);
		DrawDebugSphere(GetWorld(), player_LKP, 500, 8, FColor::Blue);

	for (const AActor* Result : m_closestInterestPoints)
	{
		if (Result)
		{
			// This is one of your spheres
			DrawDebugSphere(GetWorld(), Result->GetActorLocation(), DebugBallRadius, 8, FColor::Red);
		}
	}

}
