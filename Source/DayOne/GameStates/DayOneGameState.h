// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DayOneGameState.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ADayOneGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	FString GameMessage;
	UPROPERTY(Replicated)
	int GameOverCountDown;

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};
