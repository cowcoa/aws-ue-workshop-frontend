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
	void SetHUDScore(float Score);
	void SetHUDWeaponAmmo(int32 Ammo);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

	void PollInit();

private:
	UPROPERTY()
	class ACombatHUD* CombatHUD;

	UPROPERTY()
	class UCharacterOverlayWidget* CharacterOverlay;

	float HUDScore;
	bool bInitializeScore = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
};
