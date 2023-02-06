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
	UPROPERTY(Replicated)
	FString PlayerGameId;
	UPROPERTY(Replicated)
	FString TeamName;

	void AddToScore(float ScoreAmount);

protected:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;

private:
	UPROPERTY()
	class ADayOneCharacter* Character;
	UPROPERTY()
	class ADayOnePlayerController* Controller;
};
