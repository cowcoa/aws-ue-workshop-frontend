// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h"
#include "GameFramework/GameModeBase.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "DayOneGameModeBase.generated.h"

UENUM()
enum class EUpdateReason : uint8
{
	NO_UPDATE_RECEIVED,
	BACKFILL_INITIATED,
	MATCHMAKING_DATA_UPDATED,
	BACKFILL_FAILED,
	BACKFILL_TIMED_OUT,
	BACKFILL_CANCELLED,
	BACKFILL_COMPLETED
};

USTRUCT()
struct FStartGameSessionState
{
	GENERATED_BODY();

	UPROPERTY()
	bool Status;

	UPROPERTY()
	FString MatchmakingConfigurationArn;

	TMap<FString, Aws::GameLift::Server::Model::Player> PlayerIdToPlayer;

	FStartGameSessionState() {
		Status = false;
	}
};

USTRUCT()
struct FUpdateGameSessionState
{
	GENERATED_BODY();

	UPROPERTY()
	EUpdateReason Reason;

	TMap<FString, Aws::GameLift::Server::Model::Player> PlayerIdToPlayer;

	FUpdateGameSessionState() {
		Reason = EUpdateReason::NO_UPDATE_RECEIVED;
	}
};

USTRUCT()
struct FProcessTerminateState
{
	GENERATED_BODY();

	UPROPERTY()
	bool Status;

	long TerminationTime;

	FProcessTerminateState() {
		Status = false;
		TerminationTime = 0L;
	}
};

USTRUCT()
struct FHealthCheckState
{
	GENERATED_BODY();

	UPROPERTY()
	bool Status;

	FHealthCheckState() {
		Status = false;
	}
};

UCLASS()
class DAYONE_API ADayOneGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADayOneGameModeBase();

protected:
	virtual void BeginPlay() override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;
	
private:
	FHttpModule* HttpModule;

	//
	
	UPROPERTY()
	FTimerHandle CountDownUntilGameOverHandle;

	UPROPERTY()
	FTimerHandle EndGameHandle;

	UPROPERTY()
	FTimerHandle PickAWinningTeamHandle;

	UPROPERTY()
	FTimerHandle HandleProcessTerminationHandle;

	UPROPERTY()
	FTimerHandle HandleGameSessionUpdateHandle;

	UPROPERTY()
	FTimerHandle SuspendBackfillHandle;

	// 
	
	UPROPERTY()
	FStartGameSessionState StartGameSessionState;

	UPROPERTY()
	FUpdateGameSessionState UpdateGameSessionState;

	UPROPERTY()
	FProcessTerminateState ProcessTerminateState;

	UPROPERTY()
	FHealthCheckState HealthCheckState;

	//
	UPROPERTY()
	FString ApiUrl;

	UPROPERTY()
	FString ServerPassword;

	UPROPERTY()
	int RemainingGameTime;

	UPROPERTY()
	bool GameSessionActivated;

	UPROPERTY()
	FString LatestBackfillTicketId;

	UPROPERTY()
	bool WaitingForPlayersToJoin;

	UPROPERTY()
	int TimeSpentWaitingForPlayersToJoin;

	TMap<FString, Aws::GameLift::Server::Model::Player> ExpectedPlayers;
	
	//
	UFUNCTION()
	void CountDownUntilGameOver();
	
	UFUNCTION()
	void EndGame();

	UFUNCTION()
	void PickAWinningTeam();

	UFUNCTION()
	void HandleProcessTermination();

	UFUNCTION()
	void HandleGameSessionUpdate();

	UFUNCTION()
	void SuspendBackfill();

	//
	FString CreateBackfillRequest(FString GameSessionArn, FString MatchmakingConfigurationArn, TMap<FString, Aws::GameLift::Server::Model::Player> Players);
	bool StopBackfillRequest(FString GameSessionArn, FString MatchmakingConfigurationArn, FString TicketId);
	void OnRecordMatchResultResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
