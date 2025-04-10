// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SDTAIController.h"
#include "LoadBalancer.generated.h"

UCLASS()
class SOFTDESIGNTRAINING_API ALoadBalancer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALoadBalancer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	TArray<ASDTAIController*> p_aicontrollers;
	int p_current_index = 0;
public:	 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
	float m_MaxAllocatedTime = 2 / 1000;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void RegisterAI(ASDTAIController* controller);
	void UnregisterAI(ASDTAIController* controller);

	static ALoadBalancer& GetInstance();


};
