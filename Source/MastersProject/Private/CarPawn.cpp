// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"

#include "AICarController.h"
#include "AIController.h"
#include "Components/SplineComponent.h"
#include "CarDecisionTreeComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
ACarPawn::ACarPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	RootComponent = StaticMesh;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>("BoxComponent");
	BoxComponent->SetupAttachment(RootComponent);
	BoxComponent->ComponentTags.AddUnique("AvoidanceBox");
	//BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ACarPawn::OnEnterRange);
	//BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ACarPawn::OnExitRange);

	DecisionTree = CreateDefaultSubobject<UCarDecisionTreeComponent>("DecisionTree");
}

// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
	Super::BeginPlay();

	TArray<int> conditions;
	DecisionTree->ResetCurrent();
	conditions.Add(6);
	DecisionTree->CreateChildAtCurrent(conditions, "Root");
	conditions.Empty();
	conditions.Add(3);
	int nonPitRoot = DecisionTree->CreateChildAtCurrent(conditions, "ERROR");
	DecisionTree->SetCurrent(nonPitRoot);
	conditions.Empty();
	conditions.Add(0);
	conditions.Add(1);
	int next = DecisionTree->CreateChildAtCurrent(conditions, "ERROR");
	DecisionTree->SetCurrent(next);
	conditions.Empty();
	DecisionTree->CreateChildAtCurrent(conditions, "Hotlap");
	DecisionTree->CreateChildAtCurrent(conditions, "Overtake");
	DecisionTree->SetCurrent(nonPitRoot);
	conditions.Empty();
	conditions.Add(5);
	next = DecisionTree->CreateChildAtCurrent(conditions, "ERROR");
	DecisionTree->SetCurrent(next);
	conditions.Empty();
	DecisionTree->CreateChildAtCurrent(conditions, "Conserve");
	DecisionTree->CreateChildAtCurrent(conditions, "Pit");
	
	DecisionTree->ResetCurrent();
	DecisionTree->CreateChildAtCurrent(conditions, "InPit");
	DecisionTree->ResetCurrent();
	
	CurrentFuel = MaxFuel;
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	NearbyCars.Empty();
	TArray<UPrimitiveComponent*> OverlappingComponents;
	BoxComponent->GetOverlappingComponents(OverlappingComponents);

	for (int i = 0; i < OverlappingComponents.Num(); i++)
	{
		if (OverlappingComponents[i] != nullptr && IsValid(OverlappingComponents[i]))
		{
			if (OverlappingComponents[i]->GetOwner() != nullptr && IsValid(OverlappingComponents[i]->GetOwner()))
			{
				if (!OverlappingComponents[i]->ComponentHasTag("AvoidanceBox"))
				{
					ACarPawn* otherCar = Cast<ACarPawn>(OverlappingComponents[i]->GetOwner());
					if (otherCar != nullptr && IsValid(otherCar))
					{
						if (otherCar->GetUniqueID() != GetUniqueID())
							NearbyCars.AddUnique(otherCar);
					}
				}
			}
		}
	}

	if (CurrentFuel <= 0.0f)
	{
		CurrentThrottleInput = 0.0f;
	}

	float normalisedSpeed = CurrentSpeed / MaxSpeed;

	if (normalisedSpeed != CurrentThrottleInput)
	{
		float targetSpeed = CurrentThrottleInput * MaxSpeed;

		float diff = targetSpeed - CurrentSpeed;

		bool isPositive = (diff > 0);

		if (abs(diff) < MaxAcceleration)
			CurrentSpeed = targetSpeed;
		else
			CurrentSpeed += (isPositive ? (MaxAcceleration * MaxSpeed) : -(MaxAcceleration * MaxSpeed));
	}

	if (CurrentTurnInput != 0.0f)
	{
		AddActorWorldRotation(FRotator(0, (CurrentTurnInput * TurnPower) * FMath::Clamp(((MaxSpeed - CurrentSpeed) / MaxSpeed), 0.1f, 1.0f) * DeltaTime, 0));
	}

	SetActorLocation(GetActorLocation() + ((GetActorForwardVector() * CurrentSpeed) * DeltaTime));

	if (CurrentThrottleInput != 0.0f)
	{
			CurrentFuel -= (FuelMaxDecay * abs(CurrentThrottleInput)) * DeltaTime;
	}
}

// Called to bind functionality to input
void ACarPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACarPawn::SetThrottleInput(float Input)
{
	CurrentThrottleInput = FMath::Clamp(Input, -1.0f, 1.0f);
}

void ACarPawn::SetTurnInput(float Input)
{
	CurrentTurnInput = FMath::Clamp(Input, -1.0f, 1.0f);
}

FVector2f ACarPawn::CalculateInputs(FTransform target, ARacingLineManager* lineManager, float DeltaTime, bool shouldConserve, bool hasPitLimiter)
{
	USplineComponent* spline = lineManager->GetSpline();
	FTransform afterTarget = lineManager->GetNextNextSplineTransform(GetActorLocation());

	float throttleInput = 1.0f;
	float turnInput = 0.0f;

	FVector u = GetActorForwardVector();
	FVector v = target.GetLocation() - GetActorLocation();
	FVector rawV = v;
	v.Normalize();

	float angle = FMath::Acos(FVector::DotProduct(v, u) / (u.Length() * v.Length()));

	if (angle > 0.1f)
	{
		FVector rightPos = GetActorLocation() + (GetActorRightVector() * 10.0f);
		FVector leftPos = GetActorLocation() + (GetActorRightVector() * -10.0f);

		FVector rightDiff = target.GetLocation() - rightPos;
		FVector leftDiff = target.GetLocation() - leftPos;

		float distance = rawV.Length();
		float distPerUpdate = CurrentSpeed * DeltaTime;
		float updatesToDistance = distPerUpdate / distance;
		float turnsPerUpdate = (CurrentTurnInput * TurnPower) * FMath::Clamp(((MaxSpeed - CurrentSpeed) / MaxSpeed), 0.1f, 1.0f) * DeltaTime;
		//float turnsRequired = abs(turnsPerUpdate / (UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), target.GetLocation()).Yaw - GetActorRotation().Yaw));
		float turnsRequired = abs(turnsPerUpdate / angle);

		if (turnsRequired > updatesToDistance || turnsPerUpdate <= 0.0f)
		{
			if (turnsRequired <= 0.0f && turnsPerUpdate <= 0.0f)
				turnsRequired = 99999999999.0f;
			float diff = turnsRequired - (updatesToDistance);
			float halfway = turnsRequired - (diff / 2.0f);
			throttleInput = FMath::Clamp(CurrentThrottleInput - halfway, 0.1f, 1.0f);
		}

		if (rightDiff.Length() < leftDiff.Length())
		{
			turnInput = 1.0f;
		}
		else
		{
			turnInput = -1.0f;
		}
	}

	FVector2f inputs = FVector2f::Zero();
	inputs.X = throttleInput;
	inputs.Y = turnInput;

	TArray<AActor*> ignored;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("NonObstacle"), ignored);
	ignored.Add(this);
	FHitResult hitRight;
	bool rightHasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorRightVector() * 100.0f), 10.0f, ETraceTypeQuery::TraceTypeQuery1, false, ignored, EDrawDebugTrace::None, hitRight, true, FLinearColor::Green, FLinearColor::Red, 1.0f);
	if (hitRight.bBlockingHit)
	{
		inputs.Y = FMath::Clamp(inputs.Y, -1.0f, 0.0f);
	}

	FHitResult hitLeft;
	bool leftHasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorRightVector() * -100.0f), 10.0f, ETraceTypeQuery::TraceTypeQuery1, false, ignored, EDrawDebugTrace::None, hitLeft, true, FLinearColor::Blue, FLinearColor::Yellow, 1.0f);
	if (hitLeft.bBlockingHit)
	{
		inputs.Y = FMath::Clamp(inputs.Y, 0.0f, 1.0f);
	}
	

	if (hasPitLimiter)
	{
		inputs.X = FMath::Clamp(inputs.X, -0.3f, 0.3f);
	}
	else if (shouldConserve)
	{
		inputs.X = FMath::Clamp(inputs.X, -0.75f, 0.75f);
	}
	
	return inputs;
}

FVector2f ACarPawn::CalculateAvoidance(ARacingLineManager* lineManager, float DeltaTime)
{
	if (NearbyCars.Num() == 0)
		return FVector2f::Zero();
	
	FVector2f inputs = FVector2f::Zero();

	FRotator myRotation = GetActorRotation();

	FTransform target = lineManager->GetNextSplineTransform(GetActorLocation());

	ACarPawn* leftmostCar = nullptr;
	ACarPawn* rightmostCar = nullptr;
	float leftmostDist = -1.0f;
	float rightmostDist = -1.0f;

	float leftDist = 0;
	float rightDist = 0;

	for (ACarPawn* otherCar : NearbyCars)
	{
		if (otherCar != nullptr && IsValid(otherCar))
		{
			if (CurrentSpeed - otherCar->GetCurrentSpeed() >= 10.0f)
			{
				FVector otherPos = otherCar->GetActorLocation();
				FVector forwardPos = GetActorLocation() + (GetActorForwardVector() * 10.0f);
				FVector backwardPos = GetActorLocation() - (GetActorForwardVector() * 10.0f);
				if (FVector::Dist(otherPos, forwardPos) <= FVector::Dist(otherPos, backwardPos))
				{
					FVector rightPos = GetActorLocation() + (GetActorRightVector() * 10.0f);
					FVector leftPos = GetActorLocation() + (GetActorRightVector() * -10.0f);

					float carLeftDist = FVector::Dist(leftPos, otherCar->GetActorLocation());
					float carRightDist = FVector::Dist(rightPos, otherCar->GetActorLocation());

					leftDist += carLeftDist;
					rightDist += carRightDist;

					if (carLeftDist > leftmostDist)
					{
						leftmostDist = carLeftDist;
						leftmostCar = otherCar;
					}

					if (carRightDist > rightmostDist)
					{
						rightmostDist = carRightDist;
						rightmostCar = otherCar;
					}
				}
			}
		}
	}

	if (leftDist == 0 && rightDist == 0)
		return FVector2f(1.0f, 0.0f);

	FVector overtakeTarget = GetActorLocation();
	
	if (leftDist > rightDist || FMath::Abs(leftDist - rightDist) <= 0.0f)
	{
		if (leftmostCar != nullptr && IsValid(leftmostCar))
		{
			overtakeTarget = leftmostCar->GetActorLocation() + (lineManager->GetNearestRightVector(leftmostCar->GetActorLocation()) * -30.0f);
		}
	}
	else
	{
		if (rightmostCar != nullptr && IsValid(rightmostCar))
		{
			overtakeTarget = rightmostCar->GetActorLocation() + (lineManager->GetNearestRightVector(rightmostCar->GetActorLocation()) * 30.0f);
		}
	}

	float throttleInput = 1.0f;
	float turnInput = 0.0f;

	FVector u = GetActorForwardVector();
	FVector v = overtakeTarget - GetActorLocation();
	FVector rawV = v;
	v.Normalize();

	float angle = FMath::Acos(FVector::DotProduct(v, u) / (u.Length() * v.Length()));

	if (angle > 0.1f)
	{
		FVector rightPos = GetActorLocation() + (GetActorRightVector() * 10.0f);
		FVector leftPos = GetActorLocation() + (GetActorRightVector() * -10.0f);

		FVector rightDiff = overtakeTarget - rightPos;
		FVector leftDiff = overtakeTarget - leftPos;

		float distance = rawV.Length();
		float distPerUpdate = CurrentSpeed * DeltaTime;
		float updatesToDistance = distPerUpdate / distance;
		float turnsPerUpdate = (CurrentTurnInput * TurnPower) * FMath::Clamp(((MaxSpeed - CurrentSpeed) / MaxSpeed), 0.1f, 1.0f) * DeltaTime;
		float turnsRequired = abs(turnsPerUpdate / angle);

		if (turnsRequired > updatesToDistance || turnsPerUpdate <= 0.0f)
		{
			if (turnsRequired <= 0.0f && turnsPerUpdate <= 0.0f)
				turnsRequired = 99999999999.0f;
			float diff = turnsRequired - (updatesToDistance);
			float halfway = turnsRequired - (diff / 2.0f);
			//throttleInput = FMath::Clamp(CurrentThrottleInput - halfway, 0.1f, 1.0f);
			throttleInput = CurrentThrottleInput;
		}

		if (rightDiff.Length() < leftDiff.Length())
		{
			turnInput = 1.0f;
		}
		else
		{
			turnInput = -1.0f;
		}
	}

	inputs = FVector2f::Zero();
	inputs.X = throttleInput;
	inputs.Y = turnInput;
	
	TArray<AActor*> ignored;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("NonObstacle"), ignored);
	ignored.Add(this);
	FHitResult hitRight;
	bool rightHasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorRightVector() * 100.0f), 10.0f, ETraceTypeQuery::TraceTypeQuery1, false, ignored, EDrawDebugTrace::None, hitRight, true, FLinearColor::Green, FLinearColor::Red, 1.0f);
	if (hitRight.bBlockingHit)
	{
		inputs.Y = FMath::Clamp(inputs.Y, -1.0f, 0.0f);
	}

	FHitResult hitLeft;
	bool leftHasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorRightVector() * -100.0f), 10.0f, ETraceTypeQuery::TraceTypeQuery1, false, ignored, EDrawDebugTrace::None, hitLeft, true, FLinearColor::Blue, FLinearColor::Yellow, 1.0f);
	if (hitLeft.bBlockingHit)
	{
		inputs.Y = FMath::Clamp(inputs.Y, 0.0f, 1.0f);
	}
	
	return inputs;
}

bool ACarPawn::HasCarsToAvoid()
{
	return !NearbyCars.IsEmpty();
}

float ACarPawn::GetSlowestNearbySpeed()
{
	float slowest = 999999999.0f;
	for (ACarPawn* otherCar : NearbyCars)
	{
		if (otherCar != nullptr && IsValid(otherCar))
		{
			if (otherCar->GetCurrentSpeed() < slowest)
				slowest = otherCar->GetCurrentSpeed();
		}
	}

	return slowest;
}

float ACarPawn::GetCurrentSpeed()
{
	return CurrentSpeed;
}

FString ACarPawn::DecideNewTask()
{
	FString task = DecisionTree->MakeDecision();
	DecisionTree->ResetCurrent();
	return task;
}

bool ACarPawn::GetIsLowOnFuel()
{
	return CurrentFuel <= RefuelThreshold;
}

void ACarPawn::Refuel()
{
	CurrentFuel = MaxFuel;
}

int ACarPawn::GetCurrentTarget()
{
	AController* controller = GetController();
	if (controller != nullptr && IsValid(controller))
	{
		AAICarController* carCtr = Cast<AAICarController>(controller);
		if (carCtr != nullptr && IsValid(carCtr))
		{
			return carCtr->GetNextSplineTarget();
		}
	}
	return -1;
}

void ACarPawn::OnEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                            int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherComp->ComponentHasTag("AvoidanceBox"))
	{
		ACarPawn* otherCar = Cast<ACarPawn>(OtherActor);

		if (otherCar != nullptr && IsValid(otherCar) && otherCar != this)
		{
			NearbyCars.Add(otherCar);
		}
	}
}

void ACarPawn::OnExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherComp->ComponentHasTag("AvoidanceBox"))
	{
		ACarPawn* otherCar = Cast<ACarPawn>(OtherActor);

		if (otherCar != nullptr && IsValid(otherCar))
		{
			NearbyCars.Remove(otherCar);
		}
	}
}

