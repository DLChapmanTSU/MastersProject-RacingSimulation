// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CarDecisionTreeComponent.h"
#include "GameFramework/Pawn.h"
#include "RacingLineManager.h"
#include "Components/BoxComponent.h"
#include "CarPawn.generated.h"

UCLASS()
class MASTERSPROJECT_API ACarPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACarPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float MaxSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float MaxAcceleration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float TurnPower;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float CurrentSpeed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float CurrentTurnInput;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float CurrentThrottleInput;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float MaxFuel = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float FuelMaxDecay = 0.1f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		float RefuelThreshold = 20.0f;
		

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		UBoxComponent* BoxComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		UBoxComponent* BehindBoxComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		TArray<ACarPawn*> NearbyCars;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		TArray<ACarPawn*> BehindCars;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		UCarDecisionTreeComponent* DecisionTree;

	float CurrentFuel;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetThrottleInput(float Input);
	void SetTurnInput(float Input);
	FVector2f CalculateInputs(FTransform target, ARacingLineManager* lineManager, float DeltaTime, bool shouldConserve = false, bool hasPitLimiter = false);
	FVector2f CalculateAvoidance(FTransform defaultTarget, ARacingLineManager* lineManager, float DeltaTime);
	bool HasCarsToAvoid();
	float GetSlowestNearbySpeed();
	float GetCurrentSpeed();
	FString DecideNewTask();
	bool GetIsLowOnFuel();
	void Refuel();
	int GetCurrentTarget();
	FVector2f CheckEvasiveActions(ARacingLineManager* lineManager, float DeltaTime);

	UFUNCTION(BlueprintCallable)
		float GetCurrentFuel();

	UFUNCTION()
	void OnEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void BackOnEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void BackOnExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	bool CheckFutureOverlap(ACarPawn* OtherCar, float DeltaTime, TArray<FVector>& CollisionPoints);
};
