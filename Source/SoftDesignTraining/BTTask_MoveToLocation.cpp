#include "BTTask_MoveToLocation.h"
#include "SDTAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToLocation::UBTTask_MoveToLocation()
{
    NodeName = "MoveToLocation";
    bNotifyTick = false;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MoveToLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ASDTAIController* aiController = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
	if (!aiController)
		return EBTNodeResult::Failed;

	if (!aiController->InAir)
	{
		aiController->MoveToLocation(aiController->m_TargetPosition);
	}

	return EBTNodeResult::Succeeded;

 }
