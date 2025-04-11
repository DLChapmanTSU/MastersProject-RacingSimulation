// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "ChaseCameraController.generated.h"

/**
 * 
 */
UCLASS()
class MASTERSPROJECT_API AChaseCameraController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

	//UPROPERTY()
	//UInputMappingContext* InputMappingContext;

	//UPROPERTY()
	//class UInputAction* SwitchAction;
};
