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

FTransform ARacingLineManager::GetNextSplineTransform(FVector position)
{
	float dist = Spline->GetDistanceAlongSplineAtLocation(position, ESplineCoordinateSpace::World);
	float key = Spline->GetInputKeyAtDistanceAlongSpline(dist);
	int truncKey = trunc(key) + 1;
	if (truncKey >= Spline->GetNumberOfSplinePoints())
		truncKey = 0;
	return Spline->GetTransformAtSplineInputKey(key, ESplineCoordinateSpace::World);
}

FTransform ARacingLineManager::GetNextNextSplineTransform(FVector position)
{
	float dist = Spline->GetDistanceAlongSplineAtLocation(position, ESplineCoordinateSpace::World);
	float key = Spline->GetInputKeyAtDistanceAlongSpline(dist);
	int truncKey = trunc(key) + 2;
	if (truncKey >= Spline->GetNumberOfSplinePoints())
		truncKey = 0;
	return Spline->GetTransformAtSplineInputKey(truncKey, ESplineCoordinateSpace::World);
}

FTransform ARacingLineManager::GetSplinePoint(int i)
{
	if (i < 0 || i >= Spline->GetNumberOfSplinePoints())
		return Spline->GetTransformAtSplinePoint(0, ESplineCoordinateSpace::World);

	return Spline->GetTransformAtSplinePoint(i, ESplineCoordinateSpace::World);
}

FVector ARacingLineManager::GetNearestRightVector(FVector position)
{
	return Spline->GetRightVectorAtDistanceAlongSpline(Spline->GetDistanceAlongSplineAtLocation(position, ESplineCoordinateSpace::World), ESplineCoordinateSpace::World);
}

int ARacingLineManager::GetSplinePointCount()
{
	return Spline->GetNumberOfSplinePoints();
}

