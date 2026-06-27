// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AuraAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

AAuraAIController::AAuraAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboadComponent");
	check(Blackboard);
	BehaviorTreeComonent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComonent");
	check(BehaviorTreeComonent);


}
