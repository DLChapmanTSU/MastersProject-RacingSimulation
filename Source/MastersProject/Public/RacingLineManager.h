// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "RacingLineManager.generated.h"

UCLASS()
class MASTERSPROJECT_API ARacingLineManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARacingLineManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = RacingLineManager)
		USplineComponent* Spline;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		FTransform GetClosestSplineLocation(FVector position);

	UFUNCTION(BlueprintCallable)
		FTransform GetNextSplineTransform(FVector position);

	UFUNCTION(BlueprintCallable)
		FTransform GetNextNextSplineTransform(FVector position);
};
