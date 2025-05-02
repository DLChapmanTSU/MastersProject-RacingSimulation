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

	BehindBoxComponent = CreateDefaultSubobject<UBoxComponent>("BehindBoxComponent");
	BehindBoxComponent->SetupAttachment(RootComponent);
	BehindBoxComponent->ComponentTags.AddUnique("AvoidanceBox");
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

	BehindCars.Empty();
	TArray<UPrimitiveComponent*> BehindOverlappingComponents;
	BoxComponent->GetOverlappingComponents(BehindOverlappingComponents);

	for (int i = 0; i < BehindOverlappingComponents.Num(); i++)
	{
		if (BehindOverlappingComponents[i] != nullptr && IsValid(BehindOverlappingComponents[i]))
		{
			if (BehindOverlappingComponents[i]->GetOwner() != nullptr && IsValid(BehindOverlappingComponents[i]->GetOwner()))
			{
				if (!BehindOverlappingComponents[i]->ComponentHasTag("AvoidanceBox"))
				{
					ACarPawn* otherCar = Cast<ACarPawn>(BehindOverlappingComponents[i]->GetOwner());
					if (otherCar != nullptr && IsValid(otherCar))
					{
						if (otherCar->GetUniqueID() != GetUniqueID())
							BehindCars.AddUnique(otherCar);
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
	bool rightHasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorRightVector() * 50.0f), 10.0f, ETraceTypeQuery::TraceTypeQuery1, false, ignored, EDrawDebugTrace::None, hitRight, true, FLinearColor::Green, FLinearColor::Red, 1.0f);
	if (hitRight.bBlockingHit)
	{
		inputs.Y = FMath::Clamp(inputs.Y, -1.0f, 0.0f);
	}

	FHitResult hitLeft;
	bool leftHasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorRightVector() * -50.0f), 10.0f, ETraceTypeQuery::TraceTypeQuery1, false, ignored, EDrawDebugTrace::None, hitLeft, true, FLinearColor::Blue, FLinearColor::Yellow, 1.0f);
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

FVector2f ACarPawn::CalculateAvoidance(FTransform defaultTarget, ARacingLineManager* lineManager, float DeltaTime)
{
	if (NearbyCars.Num() == 0)
		return CalculateInputs(defaultTarget, lineManager, DeltaTime);
	
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
			if (CurrentSpeed - otherCar->GetCurrentSpeed() >= 100.0f)
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

	//if (leftDist == 0 && rightDist == 0)
	//	return CalculateInputs(defaultTarget, lineManager, DeltaTime);

	FVector overtakeTarget = GetActorLocation();

	if (NearbyCars.Num() == 1)
	{
		if (leftmostCar != nullptr && IsValid(leftmostCar))
		{
			FVector targetRight = leftmostCar->GetActorLocation() + (lineManager->GetNearestRightVector(leftmostCar->GetActorLocation()) * 10.0f);
			FVector targetLeft = leftmostCar->GetActorLocation() + (lineManager->GetNearestRightVector(leftmostCar->GetActorLocation()) * -10.0f);

			if (FVector::Dist(GetActorLocation(), targetRight) <= FVector::Dist(GetActorLocation(), targetLeft))
			{
				overtakeTarget = leftmostCar->GetActorLocation() + (lineManager->GetNearestRightVector(leftmostCar->GetActorLocation()) * 30.0f);
			}
			else
			{
				overtakeTarget = leftmostCar->GetActorLocation() + (lineManager->GetNearestRightVector(leftmostCar->GetActorLocation()) * -30.0f);
			}
		}
	}
	else if (leftDist > rightDist || FMath::Abs(leftDist - rightDist) <= 0.0f)
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
			turnInput = 0.5f;
		}
		else
		{
			turnInput = -0.5f;
		}
	}

	inputs = FVector2f::Zero();
	inputs.X = throttleInput;
	inputs.Y = turnInput;
	
	TArray<AActor*> ignored;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("NonObstacle"), ignored);
	ignored.Add(this);
	FHitResult hitRight;
	bool rightHasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorRightVector() * 50.0f), 10.0f, ETraceTypeQuery::TraceTypeQuery1, false, ignored, EDrawDebugTrace::None, hitRight, true, FLinearColor::Green, FLinearColor::Red, 1.0f);
	if (hitRight.bBlockingHit)
	{
		inputs.Y = FMath::Clamp(inputs.Y, -1.0f, 0.0f);
		if (hitRight.GetActor()->GetActorForwardVector() != GetActorForwardVector())
		{
			FVector futurePos = GetActorLocation() + GetActorForwardVector();
			FVector otherFuturePos = hitRight.GetActor()->GetActorLocation() + hitRight.GetActor()->GetActorForwardVector();

			if (FVector::Distance(futurePos, otherFuturePos) < FVector::Distance(GetActorLocation(), hitRight.GetActor()->GetActorLocation()))
			{
				inputs.Y = FMath::Clamp(inputs.Y, -1.0f, -0.5f);
			}
		}
	}

	FHitResult hitLeft;
	bool leftHasHit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorRightVector() * -50.0f), 10.0f, ETraceTypeQuery::TraceTypeQuery1, false, ignored, EDrawDebugTrace::None, hitLeft, true, FLinearColor::Blue, FLinearColor::Yellow, 1.0f);
	if (hitLeft.bBlockingHit)
	{
		inputs.Y = FMath::Clamp(inputs.Y, 0.0f, 1.0f);
		if (hitLeft.GetActor()->GetActorForwardVector() != GetActorForwardVector())
		{
			FVector futurePos = GetActorLocation() + GetActorForwardVector();
			FVector otherFuturePos = hitLeft.GetActor()->GetActorLocation() + hitLeft.GetActor()->GetActorForwardVector();

			if (FVector::Distance(futurePos, otherFuturePos) < FVector::Distance(GetActorLocation(), hitLeft.GetActor()->GetActorLocation()))
			{
				inputs.Y = FMath::Clamp(inputs.Y, 0.5f, 1.0f);
			}
		}
	}

	FVector right = GetActorLocation() + GetActorRightVector();
	FVector left = GetActorLocation() - GetActorRightVector();

	bool isRightTurn = FVector::Distance(right, defaultTarget.GetLocation()) < FVector::Distance(left, defaultTarget.GetLocation());

	if ((isRightTurn && !rightHasHit && leftHasHit) || (!isRightTurn && !leftHasHit && rightHasHit))
	{
		return CalculateInputs(defaultTarget, lineManager, DeltaTime);
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

FVector2f ACarPawn::CheckEvasiveActions(ARacingLineManager* lineManager, float DeltaTime)
{
	if (CurrentSpeed < 50.0f)
		return FVector2f(CurrentThrottleInput, CurrentTurnInput);
	
	TArray<AActor*> CarActors;
	TArray<FVector> CollisionPoints;
	TArray<ACarPawn*> CollisionCars;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACarPawn::StaticClass(), CarActors);
	for (AActor* carActor : CarActors)
	{
		ACarPawn* otherCar = Cast<ACarPawn>(carActor);

		if (otherCar != nullptr && IsValid(otherCar) && otherCar != this)
		{
			if (otherCar->GetUniqueID() != GetUniqueID())
			{
				if (CheckFutureOverlap(otherCar, DeltaTime, CollisionPoints))
				{
					CollisionCars.Add(otherCar);
				}
			}
		}
	}

	FVector2f newInputs = FVector2f::ZeroVector;
	newInputs.X = CurrentThrottleInput;
	newInputs.Y = CurrentTurnInput;
	if (CollisionPoints.Num() > 0)
	{
		if (CollisionPoints.Num() == 1)
		{
			FVector rightPos = CollisionCars[0]->GetActorLocation() + (CollisionCars[0]->GetActorRightVector() * 10.0f);
			FVector leftPos = CollisionCars[0]->GetActorLocation() + (CollisionCars[0]->GetActorRightVector() * -10.0f);

			if (FVector::Dist(rightPos, GetActorLocation()) < FVector::Dist(leftPos, GetActorLocation()))
			{
				newInputs.Y = CurrentTurnInput + 0.1f;
			}
			else
			{
				newInputs.Y = CurrentTurnInput - 0.1f;
			}
			
			FVector u = GetActorForwardVector();
			FVector v = CollisionCars[0]->GetActorLocation() - GetActorLocation();
			FVector rawV = v;
			v.Normalize();

			float angle = FMath::Acos(FVector::DotProduct(v, u) / (u.Length() * v.Length()));

			if (angle > 2.0f)
			{
				newInputs.X = CurrentThrottleInput - 0.1f;
			}
		}
		else
		{
			FVector closestPoint = FVector::ZeroVector;
			float closestDistance = 999999999.0f;
			int index = 0;
			int currentIndex = 0;

			for (FVector point : CollisionPoints)
			{
				if (FVector::Dist(point, GetActorLocation()) < closestDistance)
				{
					closestPoint = point;
					closestDistance = FVector::Dist(point, GetActorLocation());
					index = currentIndex;
				}
				currentIndex++;
			}

			if (closestDistance != 999999999.0f && CollisionCars.IsValidIndex(currentIndex))
			{
				FVector rightPos = CollisionCars[currentIndex]->GetActorLocation() + (CollisionCars[currentIndex]->GetActorRightVector() * 10.0f);
				FVector leftPos = CollisionCars[currentIndex]->GetActorLocation() + (CollisionCars[currentIndex]->GetActorRightVector() * -10.0f);

				if (FVector::Dist(rightPos, GetActorLocation()) < FVector::Dist(leftPos, GetActorLocation()))
				{
					newInputs.Y = CurrentTurnInput + 0.1f;
				}
				else
				{
					newInputs.Y = CurrentTurnInput - 0.1f;
				}
			
				FVector u = GetActorForwardVector();
				FVector v = CollisionCars[currentIndex]->GetActorLocation() - GetActorLocation();
				FVector rawV = v;
				v.Normalize();

				float angle = FMath::Acos(FVector::DotProduct(v, u) / (u.Length() * v.Length()));

				if (angle < 1.5f)
				{
					newInputs.X = CurrentThrottleInput - 0.1f;
				}
			}
		}
	}
	CurrentThrottleInput = newInputs.X;
	CurrentTurnInput = newInputs.Y;
	return newInputs;
}

float ACarPawn::GetCurrentFuel()
{
	return CurrentFuel;
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

void ACarPawn::BackOnEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherComp->ComponentHasTag("AvoidanceBox"))
	{
		ACarPawn* otherCar = Cast<ACarPawn>(OtherActor);

		if (otherCar != nullptr && IsValid(otherCar) && otherCar != this)
		{
			BehindCars.Add(otherCar);
		}
	}
}

void ACarPawn::BackOnExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (!OtherComp->ComponentHasTag("AvoidanceBox"))
	{
		ACarPawn* otherCar = Cast<ACarPawn>(OtherActor);

		if (otherCar != nullptr && IsValid(otherCar))
		{
			BehindCars.Remove(otherCar);
		}
	}
}

bool ACarPawn::CheckFutureOverlap(ACarPawn* OtherCar, float DeltaTime, TArray<FVector>& CollisionPoints)
{
	if (OtherCar != nullptr && IsValid(OtherCar))
	{
		if (BehindCars.Contains(OtherCar))
			return false;
		FVector myLocation = GetActorLocation();
		FVector otherLocation = OtherCar->GetActorLocation();

		if (FVector::Dist(myLocation, otherLocation) >= 200.0f)
			return false;

		float rotDiff = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), OtherCar->GetActorForwardVector()));
		float otherSpeed = OtherCar->GetCurrentSpeed();

		float speedDiff = CurrentSpeed > otherSpeed ? CurrentSpeed - otherSpeed : otherSpeed - CurrentSpeed;

		if (rotDiff < 0.1f && speedDiff < 10.0f)
			return false;
		
		//FVector myVelocity = (GetActorForwardVector() * CurrentSpeed) * DeltaTime;
		FVector myVelocity = GetActorForwardVector() * 100.0f;
		FVector otherVelocity = (OtherCar->GetActorForwardVector() * OtherCar->GetCurrentSpeed()) * DeltaTime;

		FVector positionDiff = myLocation - otherLocation;
		FVector velocityDiff = myVelocity - otherVelocity;

		//Quadratic substitutions
		float a = (velocityDiff.X * velocityDiff.X) + (velocityDiff.Y * velocityDiff.Y);
		float b = (2.0f * positionDiff.X * velocityDiff.X) + (2.0f * positionDiff.Y * velocityDiff.Y);
		float c = (positionDiff.X * positionDiff.X) + (positionDiff.Y * positionDiff.Y) - (50.0f * 50.0f);
		
		if ((b * b) < (4.0f * a * c))
		{
			return false;
		}

		//Calculates both possible results from the quadratic
		float positiveT = (-b + FMath::Sqrt((b * b) - (4 * a * c))) / (2 * a);
		float negativeT = (-b - FMath::Sqrt((b * b) - (4 * a * c))) / (2 * a);

		if ((positiveT >= -0.1f && positiveT <= 1.0f) || (negativeT >= -0.1f && negativeT <= 1.0f))
		{
			float usedT = 0.0f;
			if ((positiveT >= -0.1f && positiveT <= 1.0f) && (negativeT >= -0.1f && negativeT <= 1.0f))
			{
				usedT = positiveT < negativeT ? positiveT : negativeT;
			}
			else if (positiveT >= -0.1f && positiveT <= 1.0f)
			{
				usedT = positiveT;
			}
			else
			{
				usedT = negativeT;
			}

			myVelocity.Normalize();
			CollisionPoints.Add(GetActorLocation() + (myVelocity * usedT));
			return true;
		}
	}

	return false;
}

