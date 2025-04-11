// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class SOFTDESIGNTRAINING_API ASDTAIController;

class SOFTDESIGNTRAINING_API USDTLoadBalancer
{
public:
	USDTLoadBalancer() = default;

	static USDTLoadBalancer* GetInstance();

	void TickWorld(UWorld* World, ELevelTick TickType, float DeltaSeconds);

	void RegisterAI(ASDTAIController *controller);
	void UnregisterAI(ASDTAIController* controller);

	void Destroy();

	static USDTLoadBalancer* m_Instance;
	static FDelegateHandle m_handle;

private:
	TArray<ASDTAIController*> m_controllers;
	float m_MaxAllocatedTime = 2.f / 1000.f;

	int m_AgentIndex = 0;
};
