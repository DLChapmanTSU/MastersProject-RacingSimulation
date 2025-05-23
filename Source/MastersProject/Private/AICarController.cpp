// Fill out your copyright notice in the Description page of Project Settings.


#include "AICarController.h"

#include "CarGameInstance.h"
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
	{
		for (AActor* actor : found)
		{
			if (actor->ActorHasTag("IsPit"))
			{
				PitLineManager = Cast<ARacingLineManager>(actor);
			}
			else
			{
				RacingLineManager = Cast<ARacingLineManager>(actor);
			}
		}
	}

	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (GameInstance != nullptr && IsValid(GameInstance))
	{
		UCarGameInstance* CarGameInstance = Cast<UCarGameInstance>(GameInstance);
		if (CarGameInstance != nullptr && IsValid(CarGameInstance))
		{
			TotalLaps = CarGameInstance->GetTotalLaps();
		}
	}
}

// Called every frame
void AAICarController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CarPawn != nullptr && IsValid(CarPawn) && RacingLineManager != nullptr && IsValid(RacingLineManager) && PitLineManager != nullptr && IsValid(PitLineManager))
	{
		if (LapsCompleted >= TotalLaps)
		{
			CarPawn->SetThrottleInput(0);
			CarPawn->SetTurnInput(0);
			return;
		}
		FTransform targetTransform = IsFollowingPit ? PitLineManager->GetSplinePoint(NextSplineTarget) : RacingLineManager->GetSplinePoint(NextSplineTarget);
		/*FVector diff = targetTransform.GetLocation() - CarPawn->GetActorLocation();
		if (diff.Length() <= 100.0f)
		{
			NextSplineTarget++;
			if (NextSplineTarget >= RacingLineManager->GetSplinePointCount())
				NextSplineTarget = 0;

			targetTransform = RacingLineManager->GetSplinePoint(NextSplineTarget);
		}*/

		FString task = CarPawn->DecideNewTask();
		
		FRotator rotatorDiff = targetTransform.Rotator() - CarPawn->GetActorRotation();
		FVector2f inputs;

		if (task == "HotLap")
		{
			IsFollowingPit = false;
			inputs = CarPawn->CalculateAvoidance(targetTransform, RacingLineManager, DeltaTime);
		}
		else if (task == "Overtake")
		{
			IsFollowingPit = false;
			inputs = CarPawn->CalculateAvoidance(targetTransform, RacingLineManager, DeltaTime);
		}
		else if (task == "Pit")
		{
			if (IsFollowingPit == false)
			{
				IsFollowingPit = true;
				NextSplineTarget = 0;
			}
				
			inputs = CarPawn->CalculateInputs(targetTransform, PitLineManager, DeltaTime);
		}
		else if (task == "InPit")
		{
			inputs = CarPawn->CalculateInputs(targetTransform, PitLineManager, DeltaTime, true, true);
		}
		else if (task == "Conserve")
		{
			IsFollowingPit = false;
			inputs = CarPawn->CalculateInputs(targetTransform, RacingLineManager, DeltaTime, true);
		}
		else
		{
			IsFollowingPit = false;
			inputs = CarPawn->CalculateInputs(targetTransform, RacingLineManager, DeltaTime);
		}
			
		//if (task == "HotLap")
		//	inputs = CarPawn->CalculateInputs(targetTransform, RacingLineManager, DeltaTime);
		//else
		//	inputs = CarPawn->CalculateAvoidance(RacingLineManager, DeltaTime);
		CarPawn->SetThrottleInput(inputs.X);
		CarPawn->SetTurnInput(inputs.Y);

		CarPawn->CheckEvasiveActions(RacingLineManager, DeltaTime);
	}
}

int AAICarController::GetNextSplineTarget()
{
	return NextSplineTarget;
}

bool AAICarController::GetIsFollowingPits()
{
	return IsFollowingPit;
}

void AAICarController::UpdateWaypointTarget(int nextPoint)
{
	LastSplineTarget = NextSplineTarget;
	NextSplineTarget = nextPoint;

	if (IsFollowingPit)
	{
		if (NextSplineTarget >= PitLineManager->GetSplinePointCount() || NextSplineTarget < 0)
		{
			NextSplineTarget = PitReturnTarget;
			IsFollowingPit = false;
			CarPawn->Refuel();
			LapsCompleted++;
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FinishedLap");

			if (LapsCompleted >= TotalLaps)
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FinishedRace");
		}
	}
	else
	{
		if (NextSplineTarget >= RacingLineManager->GetSplinePointCount() || NextSplineTarget < 0)
			NextSplineTarget = 0;

		if (NextSplineTarget == 1 && LastSplineTarget == 0)
		{
			LapsCompleted++;
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FinishedLap");

			if (LapsCompleted >= TotalLaps)
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "FinishedRace");
		}
	}
}

int AAICarController::GetLapsCompleted()
{
	return LapsCompleted;
}

int AAICarController::GetTotalLaps()
{
	return TotalLaps;
}

