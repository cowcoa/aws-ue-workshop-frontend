// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftGameMode.h"
#include "BattlefieldGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ABattlefieldGameMode : public AGameLiftGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(class ADayOneCharacter* ElimmedCharacter, class ADayOnePlayerController* VictimController, ADayOnePlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
};
