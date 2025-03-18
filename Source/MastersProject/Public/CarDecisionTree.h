// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CarDecisionTree.generated.h"

USTRUCT()
struct FDecisionTreeNode
{
	GENERATED_BODY()
	TArray<int> Children;
	TArray<int> Conditions;
	FString DecisionKey = "None";
};

class ACarPawn;
/**
 * 
 */
UCLASS()
class MASTERSPROJECT_API UCarDecisionTree : public UObject
{
	GENERATED_BODY()
protected:
	TArray<FDecisionTreeNode> Nodes;
	int Current = 0;
	ACarPawn* CarPawn;
public:
	UCarDecisionTree();
	void SetCar(ACarPawn* NewCarPawn);
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
