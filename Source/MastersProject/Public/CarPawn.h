// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true), Category = Car)
		UBoxComponent* BoxComponent;

	TArray<ACarPawn*> NearbyCars;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetThrottleInput(float Input);
	void SetTurnInput(float Input);
	FVector2f CalculateInputs(FTransform target, ARacingLineManager* lineManager, float DeltaTime);
	FVector2f CalculateAvoidance(ARacingLineManager* lineManager, float DeltaTime);
};
