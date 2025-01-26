// Fill out your copyright notice in the Description page of Project Settings.


#include "AICarController.h"
#include "CarPawn.h"


// Sets default values
AAICarController::AAICarController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAICarController::BeginPlay()
{
	Super::BeginPlay();

	CarPawn = Cast<ACarPawn>(GetPawn());
}

// Called every frame
void AAICarController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CarPawn != nullptr && IsValid(CarPawn))
	{
		CarPawn->SetThrottleInput(0.5f);
	}
}

