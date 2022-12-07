// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DayOnePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ADayOnePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString PlayerSessionId;

	UPROPERTY()
	FString MatchmakingPlayerId;

	UPROPERTY(Replicated)
	FString Team;

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};
