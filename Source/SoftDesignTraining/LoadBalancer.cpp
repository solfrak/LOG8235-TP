// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadBalancer.h"

static ALoadBalancer* _load_balancer_instance = nullptr;

// Sets default values
ALoadBalancer::ALoadBalancer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if(!_load_balancer_instance)
		_load_balancer_instance = this;

}

// Called when the game starts or when spawned
void ALoadBalancer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALoadBalancer::Tick(float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ALoadBalancer::Tick);
	Super::Tick(DeltaTime);

	double StartTime = FPlatformTime::Seconds();
	double ElapsedTime = 0;

	while (ElapsedTime < m_MaxAllocatedTime)
	{

		auto controller = p_aicontrollers[p_current_index];

		//controller->CustomTick(DeltaTime);

		p_current_index = p_current_index % p_aicontrollers.Num();

		ElapsedTime = FPlatformTime::Seconds() - StartTime;
	}

}

void ALoadBalancer::RegisterAI(ASDTAIController* controller)
{
	p_aicontrollers.Add(controller);
}

void ALoadBalancer::UnregisterAI(ASDTAIController* controller)
{
	p_aicontrollers.Remove(controller);
}


ALoadBalancer& ALoadBalancer::GetInstance()
{
	return *_load_balancer_instance;
}



