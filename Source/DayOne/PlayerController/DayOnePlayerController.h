// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DayOnePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ADayOnePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY()
	class ACombatHUD* CombatHUD;
};
