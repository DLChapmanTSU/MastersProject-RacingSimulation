// Fill out your copyright notice in the Description page of Project Settings.


#include "AICarController.h"
#include "CarPawn.h"
#include "RacingLineManager.h"
#include "Kismet/GameplayStatics.h"


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
	TArray<AActor*> found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARacingLineManager::StaticClass(), found);
	if (found.Num() > 0)
		RacingLineManager = Cast<ARacingLineManager>(found[0]);
}

// Called every frame
void AAICarController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CarPawn != nullptr && IsValid(CarPawn) && RacingLineManager != nullptr && IsValid(RacingLineManager))
	{
		FTransform targetTransform = RacingLineManager->GetSplinePoint(NextSplineTarget);
		/*FVector diff = targetTransform.GetLocation() - CarPawn->GetActorLocation();
		if (diff.Length() <= 100.0f)
		{
			NextSplineTarget++;
			if (NextSplineTarget >= RacingLineManager->GetSplinePointCount())
				NextSplineTarget = 0;

			targetTransform = RacingLineManager->GetSplinePoint(NextSplineTarget);
		}*/
		
		if (targetTransform.GetRotation() == CarPawn->GetActorRotation().Quaternion())
		{
			CarPawn->SetThrottleInput(1.0f);
			CarPawn->SetTurnInput(0.0f);
		}
		else
		{
			FRotator rotatorDiff = targetTransform.Rotator() - CarPawn->GetActorRotation();
			FVector2f inputs;

			if (!CarPawn->HasCarsToAvoid())
				inputs = CarPawn->CalculateInputs(targetTransform, RacingLineManager, DeltaTime);
			else
				inputs = CarPawn->CalculateAvoidance(RacingLineManager, DeltaTime);
			CarPawn->SetThrottleInput(inputs.X);
			CarPawn->SetTurnInput(inputs.Y);
		}
	}
}

void AAICarController::UpdateWaypointTarget()
{
	NextSplineTarget++;
	if (NextSplineTarget >= RacingLineManager->GetSplinePointCount())
		NextSplineTarget = 0;
}

