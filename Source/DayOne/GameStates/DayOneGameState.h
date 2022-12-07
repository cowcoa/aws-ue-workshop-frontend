// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DayOneGameState.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ADayOneGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	FString LatestEvent;

	UPROPERTY(Replicated)
	FString WinningTeam;

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};
