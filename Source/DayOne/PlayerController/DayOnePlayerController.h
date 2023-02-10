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
	void SetHUDCarriedAmmo(int32 Ammo);
	// countdown game over time
	virtual float GetServerTime(); // Synced with server world clock
	void SetHUDMatchCountdown(float CountdownTime);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

	void SetHUDTime();
	void PollInit();

	/**
* Sync time between client and server
*/

	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

	float SingleTripTime = 0.f;

private:
	UPROPERTY()
	class ACombatHUD* CombatHUD;

	UPROPERTY()
	class UCharacterOverlayWidget* CharacterOverlay;

	float HUDScore;
	bool bInitializeScore = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;

	// time of the game
	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
};
