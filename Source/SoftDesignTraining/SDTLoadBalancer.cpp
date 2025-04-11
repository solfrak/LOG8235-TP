// Fill out your copyright notice in the Description page of Project Settings.


#include "SDTLoadBalancer.h"
#include "SDTAIManager.h"
#include "SDTAIController.h"

USDTLoadBalancer* USDTLoadBalancer::m_Instance = nullptr;
FDelegateHandle USDTLoadBalancer::m_handle;

USDTLoadBalancer* USDTLoadBalancer::GetInstance()
{
	if (m_Instance == nullptr)
	{
		m_Instance = new USDTLoadBalancer();
		m_handle = FWorldDelegates::OnWorldPostActorTick.AddRaw(m_Instance, &USDTLoadBalancer::TickWorld);
	}

	return m_Instance;
}

void USDTLoadBalancer::TickWorld(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{

	double StartTime = FWindowsPlatformTime::Seconds();
	double ElapsedTime = 0;


	ASDTAIManager* ai_manager = ASDTAIManager::GetInstance();
	if (ai_manager)
		ai_manager->UpdateAgentBestPosition();

	while (ElapsedTime < m_MaxAllocatedTime)
	{

		ASDTAIController* agent = m_controllers[m_AgentIndex];
		agent->UpdateAgentProperties();


		if (ai_manager)
		{
			ai_manager->AssignPositionToAgent(agent);
		}

		int prev_index = m_AgentIndex;
		m_AgentIndex = (m_AgentIndex + 1) % m_controllers.Num();


		if (prev_index > m_AgentIndex) //We processed all the agent
		{
			break;
		}
		ElapsedTime = FWindowsPlatformTime::Seconds() - StartTime;

	}
	

}

void USDTLoadBalancer::RegisterAI(ASDTAIController* controller)
{
	m_controllers.Add(controller);
}

void USDTLoadBalancer::UnregisterAI(ASDTAIController* controller)
{
	m_controllers.Remove(controller);
}

void USDTLoadBalancer::Destroy()
{
	if (m_Instance)
	{
		FWorldDelegates::OnWorldPostActorTick.Remove(m_handle);
		delete m_Instance;
		m_Instance = nullptr;
	}
}
