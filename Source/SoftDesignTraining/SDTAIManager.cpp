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

void ASDTAIManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_assignedPOIsThisFrame.Empty();

	UpdateAgentBestPosition(); //Find POIs near LKP -> m_closestInterestPoints

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
		FCollisionObjectQueryParams(ECollisionChannel::ECC_GameTraceChannel2), 
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
	if (!agent || !m_registeredAgents.Contains(agent))
		return;

	//ensure POIs near LKP are up-to-date

	bool bNeedsNewAssignment = true;
	AActor* currentAgentPOI = agent->current_interest_point;

	//check if current POI is still valid and relevant
	if (currentAgentPOI && m_closestInterestPoints.Contains(currentAgentPOI))
	{
		//Agent has a POI, and it's still near the LKP.
		//Check if agent is close enough to its POI to potentially switch targets
		float distToCurrentPOISq = FVector::DistSquared(agent->GetPawn()->GetActorLocation(), currentAgentPOI->GetActorLocation());
		float poiReachedThresholdSq = FMath::Square(150.0f); //threshold reaching POI

		if (distToCurrentPOISq > poiReachedThresholdSq)
		{
			//Keep targeting a valid POI and hasn't reached it yet
			bNeedsNewAssignment = false;
			agent->m_TargetPosition = currentAgentPOI->GetActorLocation();
			// UE_LOG(LogTemp, Log, TEXT("Agent %s sticking to POI %s"), *agent->GetName(), *currentAgentPOI->GetName());
		}
		else
		{
			//when reached the POI or very close, allow reassignment 
			bNeedsNewAssignment = true;
		}
	}
	else
	{
		//agent has no POI, or its POI is no longer near the LKP -> Needs assignment
		bNeedsNewAssignment = true;
		agent->current_interest_point = nullptr; //clear invalid POI
	}


	// --- Assign Target Position IF Needed ---
	if (bNeedsNewAssignment)
	{
		AActor* assignedPOI = nullptr;
		float minDistanceSq = TNumericLimits<float>::Max();

		//find the closest *available* POI
		if (m_closestInterestPoints.Num() > 0)
		{
			for (AActor* poi : m_closestInterestPoints)
			{
				if (poi && !m_assignedPOIsThisFrame.Contains(poi))
				{
					float distSq = FVector::DistSquared(agent->GetPawn()->GetActorLocation(), poi->GetActorLocation());
					if (distSq < minDistanceSq)
					{
						minDistanceSq = distSq;
						assignedPOI = poi;
					}
				}
			}
		}

		//POI if found, otherwise LKP
		if (assignedPOI)
		{
			agent->current_interest_point = assignedPOI;
			agent->m_TargetPosition = assignedPOI->GetActorLocation();
			m_assignedPOIsThisFrame.Add(assignedPOI);
		}
		else
		{
			agent->current_interest_point = nullptr;
			agent->m_TargetPosition = player_LKP;
		}
	}

	//Update 'is_following_player' state 
	float distToTargetSq = FVector::DistSquared(agent->GetPawn()->GetActorLocation(), agent->m_TargetPosition);
	float targetThresholdSq = FMath::Square(150.0f);

	float distToLKPSq = FVector::DistSquared(agent->GetPawn()->GetActorLocation(), player_LKP);
	float lkpThresholdSq = FMath::Square(600.0f);

	if (distToLKPSq < lkpThresholdSq && distToTargetSq < targetThresholdSq)
	{
		agent->is_following_player = true;
	}
	else
	{
		if (distToLKPSq >= FMath::Square(lkpThresholdSq * 1.5f))
		{
			agent->is_following_player = false;
		}
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


	DrawDebugSphere(world, player_LKP, DebugBallRadius, 8, FColor::Magenta);
}

void ASDTAIManager::DrawDebugClosestInterestPoint()
{
		DrawDebugSphere(GetWorld(), player_LKP, sphere_cast_radius, 8, FColor::Blue);

	for (const AActor* Result : m_closestInterestPoints)
	{
		if (Result)
		{
			DrawDebugSphere(GetWorld(), Result->GetActorLocation(), DebugBallRadius, 8, FColor::Red);
		}
	}

}
