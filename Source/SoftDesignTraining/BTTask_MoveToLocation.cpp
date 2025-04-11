#include "BTTask_MoveToLocation.h"
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
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;

    APawn* AIPawn = AICon->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return EBTNodeResult::Failed;

    FVector TargetLocation = BlackboardComp->GetValueAsVector(FName("TargetPosition"));

    // vérifier si la position est atteignable via la navmesh
    FNavLocation ProjectedLocation;
    if (!UNavigationSystemV1::GetCurrent(AICon)->ProjectPointToNavigation(TargetLocation, ProjectedLocation))
    {
        return EBTNodeResult::Failed;
    }

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalLocation(ProjectedLocation.Location);
    MoveRequest.SetAcceptanceRadius(50.f);

    FNavPathSharedPtr NavPath;
    AICon->MoveTo(MoveRequest, &NavPath);

    return EBTNodeResult::Succeeded;
}
