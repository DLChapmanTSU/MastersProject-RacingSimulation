// Fill out your copyright notice in the Description page of Project Settings.


#include "CarDecisionTreeComponent.h"
#include "CarPawn.h"


// Sets default values for this component's properties
UCarDecisionTreeComponent::UCarDecisionTreeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCarDecisionTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCarDecisionTreeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCarDecisionTreeComponent::ResetCurrent()
{
	Current = 0;
}

void UCarDecisionTreeComponent::SetCurrent(int NewCurrent)
{
	if (NewCurrent >= 0 && NewCurrent < Nodes.Num())
		Current = NewCurrent;
}

int UCarDecisionTreeComponent::CreateChildAtCurrent(TArray<int>& cons,FString key)
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

bool UCarDecisionTreeComponent::CheckIsCurrentLeaf()
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

FString UCarDecisionTreeComponent::MakeDecision()
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

bool UCarDecisionTreeComponent::CheckCondition(int condition)
{
	if (condition < 0 || condition >= 5)
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

bool UCarDecisionTreeComponent::HasNearbyCars()
{
	ACarPawn* car = Cast<ACarPawn>(GetOwner());
	if (car != nullptr && IsValid(car))
		return car->HasCarsToAvoid();
	
	return false;
}

bool UCarDecisionTreeComponent::IsFasterThanNearbyCars()
{
	ACarPawn* car = Cast<ACarPawn>(GetOwner());
	if (car != nullptr && IsValid(car))
		return car->GetSlowestNearbySpeed() < car->GetCurrentSpeed();
	return false;
}

bool UCarDecisionTreeComponent::IsNearACorner()
{
	return false;
}

bool UCarDecisionTreeComponent::IsLowOnFuel()
{
	ACarPawn* car = Cast<ACarPawn>(GetOwner());
	if (car != nullptr && IsValid(car))
		return car->GetIsLowOnFuel();
	return false;
}

bool UCarDecisionTreeComponent::IsNearEndOfRace()
{
	return false;
}