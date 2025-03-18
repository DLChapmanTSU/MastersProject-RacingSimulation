// Fill out your copyright notice in the Description page of Project Settings.


#include "CarDecisionTree.h"
#include "CarPawn.h"

UCarDecisionTree::UCarDecisionTree()
{
}

void UCarDecisionTree::SetCar(ACarPawn* NewCarPawn)
{
	CarPawn = NewCarPawn;
}

void UCarDecisionTree::ResetCurrent()
{
	Current = 0;
}

void UCarDecisionTree::SetCurrent(int NewCurrent)
{
	if (NewCurrent >= 0 && NewCurrent < Nodes.Num())
		Current = NewCurrent;
}

int UCarDecisionTree::CreateChildAtCurrent(TArray<int>& cons,FString key)
{
	FDecisionTreeNode newNode;
	newNode.Conditions = cons;
	newNode.DecisionKey = key;

	if (Nodes.IsValidIndex(Current))
	{
		if (Nodes[Current].Children.Num() < 2)
		{
			Nodes.Add(newNode);
			int index = Nodes.Num() - 1;
			Nodes[Current].Children.Add(index);
			return index;
		}
	}
	else if (Nodes.Num() == 0)
	{
		Nodes.Add(newNode);
		return 0;
	}

	return -1;
}

bool UCarDecisionTree::CheckIsCurrentLeaf()
{
	if (Nodes.IsValidIndex(Current))
	{
		return Nodes[Current].Children.IsEmpty() || Nodes[Current].Children.Num() < 2 || Nodes[Current].Conditions.IsEmpty();
	}
	else
	{
		return true;
	}
}

FString UCarDecisionTree::MakeDecision()
{
	if (Nodes.IsValidIndex(Current))
	{
		if (CheckIsCurrentLeaf())
		{
			return Nodes[Current].DecisionKey;
		}

		bool result = true;

		for (int i = 0; i < Nodes[Current].Conditions.Num(); i++)
		{
			if (Nodes[Current].Conditions[i] < 5)
			{
				result = (result && CheckCondition(Nodes[Current].Conditions[i]));
			}

			if (result == false)
				break;
		}

		if (result == true)
			Current = Nodes[Current].Children[1];
		else
			Current = Nodes[Current].Children[0];

		return MakeDecision();
	}
	else
	{
		return "ERROR";
	}
}

bool UCarDecisionTree::CheckCondition(int condition)
{
	if (condition <= 0 || condition >= 5)
		return false;

	switch (condition)
	{
	case 0:
		return HasNearbyCars();
	case 1:
		return IsFasterThanNearbyCars();
	case 2:
		return IsNearACorner();
	case 3:
		return IsLowOnFuel();
	case 4:
		return IsNearEndOfRace();
	default:
		return false;
	}
}

bool UCarDecisionTree::HasNearbyCars()
{
	if (CarPawn != nullptr && IsValid(CarPawn))
		return CarPawn->HasCarsToAvoid();
	
	return false;
}

bool UCarDecisionTree::IsFasterThanNearbyCars()
{
	if (CarPawn != nullptr && IsValid(CarPawn))
		return CarPawn->GetSlowestNearbySpeed() < CarPawn->GetCurrentSpeed();
	return false;
}

bool UCarDecisionTree::IsNearACorner()
{
	return false;
}

bool UCarDecisionTree::IsLowOnFuel()
{
	return false;
}

bool UCarDecisionTree::IsNearEndOfRace()
{
	return false;
}
