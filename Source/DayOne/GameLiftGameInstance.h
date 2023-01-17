// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameLiftGameInstance.generated.h"

DECLARE_DELEGATE_OneParam(FOnStartGameSessionDelegate, const struct FStartGameSessionStateNew& /*StartGameSessionState*/);
DECLARE_DELEGATE_OneParam(FOnProcessTerminateDelegate, const struct FProcessTerminateStateNew& /*ProcessTerminateState*/);

USTRUCT()
struct FGameLiftPlayer
{
	GENERATED_BODY()

	FString PlayerId;
	FString TeamName;
};

USTRUCT()
struct FHealthCheckStateNew
{
	GENERATED_BODY();
	
	bool bIsHealthy;

	FHealthCheckStateNew() {
		bIsHealthy = false;
	}
};

USTRUCT()
struct FStartGameSessionStateNew
{
	GENERATED_BODY();
	
	bool bIsSuccess;
	TMap<FString, FGameLiftPlayer> PlayerMap;

	FOnStartGameSessionDelegate OnStartGameSession;

	FStartGameSessionStateNew()
	{
		bIsSuccess = false;
	}
};

USTRUCT()
struct FProcessTerminateStateNew
{
	GENERATED_BODY();
	
	bool bIsTerminating;
	long TerminationTime;

	FOnProcessTerminateDelegate OnProcessTerminate;

	FProcessTerminateStateNew()
	{
		bIsTerminating = false;
		TerminationTime = 0L;
	}
};

/**
 * 
 */
UCLASS()
class DAYONE_API UGameLiftGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void OnStart() override;

	FHealthCheckStateNew HealthCheckState;
	FStartGameSessionStateNew StartGameSessionState;
	FProcessTerminateStateNew ProcessTerminateState;
};
