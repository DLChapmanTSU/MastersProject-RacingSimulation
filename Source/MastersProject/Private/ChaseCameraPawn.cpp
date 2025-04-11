// Fill out your copyright notice in the Description page of Project Settings.


#include "ChaseCameraPawn.h"

#include "CarPawn.h"
#include "ChaseCameraController.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AChaseCameraPawn::AChaseCameraPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AChaseCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	AChaseCameraController* controller = Cast<AChaseCameraController>(GetController());
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(controller->Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (!InputMapping.IsNull())
			{
				InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), controller->InputPriority);
			}
		}
	}

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACarPawn::StaticClass(), Cars);
}

// Called every frame
void AChaseCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Cars.IsValidIndex(CurrentCar))
	{
		if (Cars[CurrentCar] != nullptr && IsValid(Cars[CurrentCar]))
		{
			FVector NewPos = Cars[CurrentCar]->GetActorLocation() + (Cars[CurrentCar]->GetActorForwardVector() * -100.0f);
			NewPos.Z += 50.0f;

			SetActorLocation(NewPos);
			SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Cars[CurrentCar]->GetActorLocation()));
		}
	}
}

// Called to bind functionality to input
void AChaseCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	Input->BindAction(SwitchInputAction, ETriggerEvent::Triggered, this, &AChaseCameraPawn::Switch);
}

void AChaseCameraPawn::Switch(const struct FInputActionValue& ActionValue)
{
	CurrentCar++;
	if (CurrentCar >= Cars.Num())
		CurrentCar = 0;
}

