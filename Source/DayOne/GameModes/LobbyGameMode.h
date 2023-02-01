// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftGameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ALobbyGameMode : public AGameLiftGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
