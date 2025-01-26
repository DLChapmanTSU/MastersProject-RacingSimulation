// Fill out your copyright notice in the Description page of Project Settings.


#include "RacingLineManager.h"

// Sets default values
ARacingLineManager::ARacingLineManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
	Spline->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ARacingLineManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARacingLineManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FTransform ARacingLineManager::GetClosestSplineLocation(FVector position)
{
	return Spline->FindTransformClosestToWorldLocation(position, ESplineCoordinateSpace::World);
}

