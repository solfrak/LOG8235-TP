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

void ASDTAIManager::DisbandGroup() {
	TSet<ASDTAIController*> AgentsCopy = m_registeredAgents;
	for (ASDTAIController* agent : AgentsCopy)
	{
		if (agent)
		{
			agent->LeavePursuitGroup();
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

//void ASDTAIManager::AssignPositionToAgent(ASDTAIController* agent)
//{
//	if (!m_registeredAgents.Contains(agent))
//		return;
//
//	if (!agent->is_following_player && m_closestInterestPoints.Num() > 0 && !m_closestInterestPoints.Contains(agent->current_interest_point))
//	{
//		UE_LOG(LogTemp, Warning, TEXT("Agent go to random intereste point"));
//		int index = round_robbin_assignation++ % m_closestInterestPoints.Num();
//		agent->current_interest_point = m_closestInterestPoints[index];
//		agent->m_TargetPosition = agent->current_interest_point->GetActorLocation();
//	}
//
//	if (agent->current_interest_point != nullptr)
//	{
//		float dist_to_interest_point = FVector::Dist(agent->GetPawn()->GetActorLocation(), agent->current_interest_point->GetActorLocation());
//		float threshold = 100.f;
//
//		float dist_to_LKP = FVector::Dist(agent->GetPawn()->GetActorLocation(), player_LKP);
//		float threshold2 = 500.f;
//
//		if (dist_to_interest_point < threshold && dist_to_LKP < threshold2)
//		{
//			agent->is_following_player = true;
//		}
//		else {
//			agent->is_following_player = false;
//		}
//	}
//
//	if (agent->is_following_player)
//	{
//
//		agent->m_TargetPosition = player_LKP;
//	}
//}

void ASDTAIManager::AssignPositionToAgent(ASDTAIController* agent)
{
	if (!agent || !m_registeredAgents.Contains(agent)) // Added null check for agent
		return;

	// Ensure POIs are up-to-date (assuming UpdateAgentBestPosition was called recently in Manager Tick)
	if (m_closestInterestPoints.Num() == 0)
	{
		// No POIs near LKP, everyone chases LKP directly
		agent->current_interest_point = nullptr;
		agent->m_TargetPosition = player_LKP;
		agent->is_following_player = true; // Assume following if chasing LKP
		return;
	}

	// --- Logic to assign only N closest agents to POIs ---
	const int MaxAgentsOnPOIs = 1; // How many agents should target POIs?
	TArray<TPair<float, ASDTAIController*>> AgentDistances; // Store distance squared and agent

	// Calculate squared distances for all agents (more efficient than Sqrt)
	for (ASDTAIController* registeredAgent : m_registeredAgents)
	{
		if (registeredAgent && registeredAgent->GetPawn())
		{
			float distSq = FVector::DistSquared(registeredAgent->GetPawn()->GetActorLocation(), player_LKP);
			AgentDistances.Add(TPair<float, ASDTAIController*>(distSq, registeredAgent));
		}
	}

	// Sort agents by distance (closest first)
	AgentDistances.Sort([](const TPair<float, ASDTAIController*>& A, const TPair<float, ASDTAIController*>& B) {
		return A.Key < B.Key;
		});

	// Determine if the current agent is one of the closest N agents
	bool bIsClosestAgent = false;
	int agentRank = -1;
	for (int i = 0; i < AgentDistances.Num(); ++i)
	{
		if (AgentDistances[i].Value == agent)
		{
			agentRank = i;
			break;
		}
	}

	if (agentRank != -1 && agentRank < MaxAgentsOnPOIs)
	{
		bIsClosestAgent = true;
	}

	// --- Assign Target Position ---
	if (bIsClosestAgent)
	{
		// This agent is one of the chosen few, assign a POI round-robin
		// Use a separate index for POI assignment among the closest agents if needed,
		// but global round-robin might be acceptable for simplicity.
		int poiIndex = round_robbin_assignation++ % m_closestInterestPoints.Num();
		AActor* assignedPOI = m_closestInterestPoints[poiIndex];

		agent->current_interest_point = assignedPOI;
		agent->m_TargetPosition = assignedPOI->GetActorLocation();
	}
	else
	{
		// This agent is not one of the closest, target LKP directly
		agent->current_interest_point = nullptr;
		agent->m_TargetPosition = player_LKP;
	}

	//Base it on proximity to the actual target (either POI or LKP)
	float distToTargetSq = FVector::DistSquared(agent->GetPawn()->GetActorLocation(), agent->m_TargetPosition);
	float targetThresholdSq = FMath::Square(150.0f); 

	float distToLKPSq = FVector::DistSquared(agent->GetPawn()->GetActorLocation(), player_LKP);
	float lkpThresholdSq = FMath::Square(600.0f); //threshold to LKP area

	//Considered following if reasonably close to LKP area AND getting close to its specific target
	if (distToLKPSq < lkpThresholdSq && distToTargetSq < targetThresholdSq)
	{
		agent->is_following_player = true;
	}
	else
	{
		// Stop "following" only if significantly far from LKP
		if (distToLKPSq >= FMath::Square(lkpThresholdSq * 1.5f))
		{
			agent->is_following_player = false;
		}
		// Otherwise, keep current state (prevents flip-flopping if moving between POI and LKP targets)
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
