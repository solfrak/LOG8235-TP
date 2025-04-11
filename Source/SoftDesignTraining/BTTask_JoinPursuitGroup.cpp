#include "BTTask_JoinPursuitGroup.h"
#include "SDTAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_JoinPursuitGroup::UBTTask_JoinPursuitGroup()
{
    NodeName = "JoinPursuitGroup";
}

EBTNodeResult::Type UBTTask_JoinPursuitGroup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ASDTAIController* AICon = Cast<ASDTAIController>(OwnerComp.GetAIOwner());
    if (!AICon)
        return EBTNodeResult::Failed;

    AICon->JoinPursuitGroup();

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsBool(FName("IsInGroup"), true);
    }

    return EBTNodeResult::Succeeded;
}
