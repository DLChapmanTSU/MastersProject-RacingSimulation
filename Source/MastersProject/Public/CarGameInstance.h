// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CarGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class MASTERSPROJECT_API UCarGameInstance : public UGameInstance
{
	GENERATED_BODY()
protected:
	int TotalLaps = 10;
public:
	UFUNCTION(BlueprintCallable)
		int GetTotalLaps();
};
