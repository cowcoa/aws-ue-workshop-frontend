// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftGameMode.h"
#include "BattlefieldGameMode.generated.h"

namespace MatchState
{
	extern DAYONE_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

/**
 * 
 */
UCLASS()
class DAYONE_API ABattlefieldGameMode : public AGameLiftGameMode
{
	GENERATED_BODY()

public:
	ABattlefieldGameMode();
	
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ADayOneCharacter* ElimmedCharacter, class ADayOnePlayerController* VictimController, ADayOnePlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	float LevelStartingTime = 0.f;

protected:
    virtual void BeginPlay() override;
    virtual void OnMatchStateSet() override;

private:
    float CountdownTime = 0.f;
	
public:
    FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
