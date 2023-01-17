// Fill out your copyright notice in the Description page of Project Settings.

#include "GameLiftGameInstance.h"
#include "GameLiftServerSDK.h"

void UGameLiftGameInstance::OnStart()
{
	Super::OnStart();

#if WITH_GAMELIFT
	// Init & Register to GameLift service
	auto InitSDKOutcome = Aws::GameLift::Server::InitSDK();
	if (InitSDKOutcome.IsSuccess())
	{
		// OnHealthCheck callback
		auto OnHealthCheck = [](OUT void* State)
		{
			UE_LOG(LogTemp, Warning, TEXT("OnHealthCheck"));
			
			FHealthCheckStateNew* OutputState = (FHealthCheckStateNew*)State;
			OutputState->bIsHealthy = true;

			return OutputState->bIsHealthy;
		};
		
		// OnStartGameSession callback
		auto OnStartGameSession = [](Aws::GameLift::Server::Model::GameSession GameSession, OUT void* State)
		{
			FStartGameSessionStateNew* OutputState = (FStartGameSessionStateNew*)State;

			// https://docs.aws.amazon.com/gamelift/latest/flexmatchguide/match-server.html#match-server-data
			FString MatchmakerData = GameSession.GetMatchmakerData();
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(MatchmakerData);
			if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
			{
				TArray<TSharedPtr<FJsonValue>> Teams = JsonObject->GetArrayField("teams");
				for (TSharedPtr<FJsonValue> Team : Teams)
				{
					TSharedPtr<FJsonObject> TeamObject = Team->AsObject();
					FString TeamName = TeamObject->GetStringField("name");

					TArray<TSharedPtr<FJsonValue>> Players = TeamObject->GetArrayField("players");
					for (TSharedPtr<FJsonValue> Player : Players)
					{
						TSharedPtr<FJsonObject> PlayerObject = Player->AsObject();
						FString PlayerId = PlayerObject->GetStringField("playerId");

						FGameLiftPlayer GameSessionPlayer;
						GameSessionPlayer.TeamName = TeamName;
						GameSessionPlayer.PlayerId = PlayerId;
						OutputState->PlayerMap.Add(PlayerId, GameSessionPlayer);
					}
				}

				auto ActiveGameSessionOutcome = Aws::GameLift::Server::ActivateGameSession();
				OutputState->bIsSuccess = ActiveGameSessionOutcome.IsSuccess();
				if (OutputState->bIsSuccess)
				{
					OutputState->OnStartGameSession.ExecuteIfBound(*OutputState);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to ActivateGameSession"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to deserialize GameSession's MatchmakerData: %s"), *MatchmakerData);
			}
		};

		// OnProcessTerminate callback
		auto OnProcessTerminate = [](OUT void* State)
		{
			FProcessTerminateStateNew* OutputState = (FProcessTerminateStateNew*)State;
			OutputState->bIsTerminating = true;

			auto TerminationTimeOutcome = Aws::GameLift::Server::GetTerminationTime();
			if (TerminationTimeOutcome.IsSuccess())
			{
				OutputState->TerminationTime = TerminationTimeOutcome.GetResult();
			}

			OutputState->OnProcessTerminate.ExecuteIfBound(*OutputState);
		};

		// Extract port from cmdline arguments.
		TArray<FString> CmdLineTokens;
		TArray<FString> CmdLineSwitches;
		int Port = FURL::UrlConfig.DefaultPort;
		// DayOneServer.exe token -port=7777
		FCommandLine::Parse(FCommandLine::Get(), CmdLineTokens, CmdLineSwitches);
		for (FString Str : CmdLineSwitches)
		{
			FString Key;
			FString Value;

			if (Str.Split("=", &Key, &Value))
			{
				if (Key.Equals("port"))
				{
					Port = FCString::Atoi(*Value);
				}
			}
		}

		// Setup logfile path.
		const char* LogFile = "DayOneGameLift.log";
		const char** LogFiles = &LogFile;
		auto LogParams = new Aws::GameLift::Server::LogParameters(LogFiles, 1);

		auto ProcessParams = new Aws::GameLift::Server::ProcessParameters(
			OnStartGameSession,
			&StartGameSessionState,
			nullptr,
			nullptr,
			OnProcessTerminate,
			&ProcessTerminateState,
			OnHealthCheck,
			&HealthCheckState,
			Port,
			*LogParams
		);

		// Notify GameLift service the game server is ready.
		auto ProcessReadyOutcome = Aws::GameLift::Server::ProcessReady(*ProcessParams);
		if (!ProcessReadyOutcome.IsSuccess())
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to call GameLift ProcessReady"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to init GameLiftServerSDK"));
	}
#endif
}
