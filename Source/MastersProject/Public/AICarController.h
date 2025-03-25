// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "AICarController.generated.h"

class ACarPawn;
class ARacingLineManager;

UCLASS()
class MASTERSPROJECT_API AAICarController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAICarController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	ACarPawn* CarPawn;
	ARacingLineManager* RacingLineManager;
	ARacingLineManager* PitLineManager;
	bool IsFollowingPit = false;

	int NextSplineTarget = 1;
	int PitReturnTarget = 2;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int GetNextSplineTarget();

	UFUNCTION(BlueprintCallable)
		void UpdateWaypointTarget(int target);
};
