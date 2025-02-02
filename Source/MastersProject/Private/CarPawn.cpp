// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"
#include "Components/SplineComponent.h"


// Sets default values
ACarPawn::ACarPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	StaticMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACarPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACarPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		AddActorWorldRotation(FRotator(0, (CurrentTurnInput * TurnPower) * (CurrentSpeed / MaxSpeed) * DeltaTime, 0));
	}

	SetActorLocation(GetActorLocation() + ((GetActorForwardVector() * CurrentSpeed) * DeltaTime));
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

FVector2f ACarPawn::CalculateInputs(FTransform target, ARacingLineManager* lineManager, float DeltaTime)
{
	USplineComponent* spline = lineManager->GetSpline();
	FTransform afterTarget = lineManager->GetNextNextSplineTransform(GetActorLocation());

	/*float targetDist = spline->GetDistanceAlongSplineAtLocation(target.GetLocation(), ESplineCoordinateSpace::World);
	float currentDist = spline->GetDistanceAlongSplineAtLocation(GetActorLocation(), ESplineCoordinateSpace::World);

	float distToTarget = targetDist - currentDist;

	if (distToTarget < 0.0f)
	{
		distToTarget = targetDist + (spline->GetSplineLength() - currentDist);
	}

	float turnToTarget = abs(abs(afterTarget.Rotator().Yaw) - abs(spline->FindTransformClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World).Rotator().Yaw));

	float distFromPointToPoint = spline->GetDistanceAlongSplineAtLocation(afterTarget.GetLocation(), ESplineCoordinateSpace::World) - targetDist;
	float updatesAtCurrentSpeed = (CurrentSpeed * DeltaTime) / distFromPointToPoint;

	float updatesAtMaxSpeed = (MaxSpeed * DeltaTime) / distToTarget;
	float updatesAtMaxTurn = (TurnPower * DeltaTime) / turnToTarget;

	float throttleInput = 0.0f;
	float turnInput = 0.0f;

	if (updatesAtCurrentSpeed < updatesAtMaxTurn && updatesAtCurrentSpeed != 0.0f)
	{
		throttleInput = throttleInput - MaxAcceleration;
		if (currentDist > 25.0f)
			turnInput = 1.0f;
		else
			turnInput = 0.0f;
	}
	else
	{
		throttleInput = 1.0f;
	}*/

	float throttleInput = 1.0f;
	float turnInput = 0.0f;

	FVector u = GetActorForwardVector();
	FVector v = target.GetLocation() - GetActorLocation();
	v.Normalize();

	float angle = FMath::Acos(FVector::DotProduct(v, u) / (u.Length() * v.Length()));

	if (angle > 0.1f)
	{
		FVector rightPos = GetActorLocation() + (GetActorRightVector() * 10.0f);
		FVector leftPos = GetActorLocation() + (GetActorRightVector() * -10.0f);

		FVector rightDiff = target.GetLocation() - rightPos;
		FVector leftDiff = target.GetLocation() - leftPos;

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
	
	return inputs;
}

