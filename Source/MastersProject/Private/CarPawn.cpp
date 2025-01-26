// Fill out your copyright notice in the Description page of Project Settings.


#include "CarPawn.h"


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
		AddActorWorldRotation(FRotator(0, (CurrentTurnInput * TurnPower) * (CurrentSpeed / MaxSpeed), 0));
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

