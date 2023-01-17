// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DayOne/GameLiftGameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "GameLiftGameMode.generated.h"

UENUM()
enum class EGameSessionState : uint8
{
	EGST_Waiting,
	EGST_Running,
	EGST_Terminating,
};

/**
 * 
 */
UCLASS()
class DAYONE_API AGameLiftGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGameLiftGameMode();

protected:
	virtual void BeginPlay() override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;

private:
	// Max game time
	EGameSessionState GameSessionState;
	int RemainingGameTime;
	
	// Timer
	FTimerHandle GuardGameSessionHandle;
	UFUNCTION()
	void GuardGameSession();
	
	FTimerHandle CountdownToGameOverHandle;
	UFUNCTION()
	void CountdownToGameOver();
	
	FTimerHandle ProcessTerminationHandle;
	UFUNCTION()
	void ProcessTermination();
	
	FTimerHandle EndGameHandle;
	UFUNCTION()
	void EndGame();

	// Callback
	FStartGameSessionStateNew StartGameSessionState;
	void OnStartGameSession(const FStartGameSessionStateNew& State);
	FProcessTerminateStateNew ProcessTerminateState;
	void OnProcessTerminate(const FProcessTerminateStateNew& State);
};