// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CarDecisionTreeComponent.generated.h"

USTRUCT()
struct FDecisionTreeNode
{
	GENERATED_BODY()
	TArray<int> Children;
	TArray<int> Conditions;
	FString DecisionKey = "None";
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASTERSPROJECT_API UCarDecisionTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCarDecisionTreeComponent();

protected:
	TArray<FDecisionTreeNode> Nodes;
	int Current = 0;
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void ResetCurrent();
	void SetCurrent(int NewCurrent);
	int CreateChildAtCurrent(TArray<int>& cons,FString key);
	bool CheckIsCurrentLeaf();
	FString MakeDecision();

	bool CheckCondition(int condition);
	bool HasNearbyCars();
	bool IsFasterThanNearbyCars();
	bool IsNearACorner();
	bool IsLowOnFuel();
	bool IsNearEndOfRace();
};
