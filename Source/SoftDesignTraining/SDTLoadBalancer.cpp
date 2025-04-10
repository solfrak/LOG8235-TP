// Fill out your copyright notice in the Description page of Project Settings.


#include "SDTLoadBalancer.h"
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
	for (auto controller : m_controllers)
	{
		controller->CalculateLineOfSight();
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
