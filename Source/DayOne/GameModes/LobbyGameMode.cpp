// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "DayOne/DayOne.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	UE_LOG(LogDayOne, Warning, TEXT("Number of players joined: %d"), NumberOfPlayers);
	
	if (NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Developers/Cow/ThirdPersonTemplate/ThirdPerson/Maps/ThirdPersonMap?listen"));
		}
	}
}
