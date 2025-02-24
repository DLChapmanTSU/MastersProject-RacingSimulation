// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetMathLibrary.h"


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

	NearbyCars.Empty();

	TArray<AActor*> OverlappingActors;
	BoxComponent->GetOverlappingActors(OverlappingActors);

	for (int i = 0; i < OverlappingActors.Num(); i++)
	{
		ACarPawn* otherCar = Cast<ACarPawn>(OverlappingActors[i]);
		if (otherCar != nullptr && IsValid(otherCar))
		{
			if (otherCar->GetUniqueID() != GetUniqueID())
				NearbyCars.AddUnique(otherCar);
		}
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
		float turnsRequired = abs(turnsPerUpdate / (UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), target.GetLocation()).Yaw - GetActorRotation().Yaw));

		if (turnsRequired > updatesToDistance || turnsPerUpdate <= 0.0f)
		{
			if (turnsRequired <= 0.0f && turnsPerUpdate <= 0.0f)
				turnsRequired = 99999999999.0f;
			float diff = turnsRequired - (updatesToDistance);
			float halfway = turnsRequired - (diff / 2.0f);
			throttleInput = FMath::Clamp(CurrentThrottleInput - halfway, 0.1f, 1.0f);
			//halfway *= distance;
			//halfway /= DeltaTime;
			//throttleInput = halfway;
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
	/*else
	{
		FVector rightPos = GetActorLocation() + (GetActorRightVector() * 10.0f);
		FVector leftPos = GetActorLocation() + (GetActorRightVector() * -10.0f);

		FVector rightDiff = target.GetLocation() - rightPos;
		FVector leftDiff = target.GetLocation() - leftPos;

		float distance = rawV.Length();
		float distPerUpdate = CurrentSpeed * DeltaTime;
		float updatesToDistance = distPerUpdate / distance;
		float turnsPerUpdate = (CurrentTurnInput * TurnPower) * FMath::Clamp(((MaxSpeed - CurrentSpeed) / MaxSpeed), 0.0f, 1.0f) * DeltaTime;
		float turnsRequired = abs(turnsPerUpdate / (UKismetMathLibrary::FindLookAtRotation(target.GetLocation(), afterTarget.GetLocation()).Yaw - GetActorRotation().Yaw));
	}*/

	/*FVector targetV = afterTarget.GetLocation() - target.GetLocation();
	targetV.Normalize();
	float targetAngle = FMath::Acos(FVector::DotProduct(targetV, v) / (v.Length() * targetV.Length()));

	if (targetAngle > 0.1f)
	{
		FVector rightPos = GetActorLocation() + (GetActorRightVector() * 10.0f);
		FVector leftPos = GetActorLocation() + (GetActorRightVector() * -10.0f);

		FVector rightDiff = target.GetLocation() - rightPos;
		FVector leftDiff = target.GetLocation() - leftPos;

		float distance = targetV.Length();
		float distPerUpdate = CurrentSpeed * DeltaTime;
		float updatesToDistance = distPerUpdate / distance;
		float turnsPerUpdate = (CurrentTurnInput * TurnPower) * (CurrentSpeed / MaxSpeed) * DeltaTime;
		float turnsRequired = turnsPerUpdate / (target.Rotator().Yaw - GetActorRotation().Yaw);

		if (turnsRequired * 100.0f > updatesToDistance)
		{
			throttleInput = CurrentThrottleInput / 2.0f;
		}

		if (rightDiff.Length() < leftDiff.Length())
		{
			turnInput = 1.0f;
		}
		else
		{
			turnInput = -1.0f;
		}
	}*/

	FVector2f inputs = FVector2f::Zero();
	inputs.X = throttleInput;
	inputs.Y = turnInput;
	
	return inputs;
}

FVector2f ACarPawn::CalculateAvoidance(ARacingLineManager* lineManager, float DeltaTime)
{
	FVector2f inputs = FVector2f::Zero();

	FRotator myRotation = GetActorRotation();

	FTransform target = lineManager->GetNextSplineTransform(GetActorLocation());

	float leftDist = 0;
	float rightDist = 0;

	for (ACarPawn* otherCar : NearbyCars)
	{
		if (otherCar != nullptr && IsValid(otherCar))
		{
			FVector otherPos = otherCar->GetActorLocation();
			if (FVector::Dist(otherPos, target.GetLocation()) <= FVector::Dist(GetActorLocation(), target.GetLocation()))
			{
				FVector rightPos = GetActorLocation() + (GetActorRightVector() * 10.0f);
				FVector leftPos = GetActorLocation() + (GetActorRightVector() * -10.0f);

				leftDist += FVector::Dist(leftPos, otherCar->GetActorLocation());
				rightDist += FVector::Dist(rightPos, otherCar->GetActorLocation());
			}
		}
	}

	if (FMath::Abs(leftDist - rightDist) <= 0.0f)
	{
		return FVector2f::Zero();
	}
	else if (leftDist > rightDist)
	{
		return FVector2f(CurrentThrottleInput, -0.75f);
	}
	else
	{
		return FVector2f(CurrentThrottleInput, 0.75f);
	}
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

