// Fill out your copyright notice in the Description page of Project Settings.


#include "DayOneWidget.h"

#include "GameLiftClientModule.h"
#include "Components/TextBlock.h"
#include "DayOne/DayOneGameInstance.h"
#include "DayOne/GameStates/DayOneGameState.h"
#include "DayOne/PlayerStates/DayOnePlayerState.h"

void UDayOneWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GLClientModule = &FGameLiftClientModule::Get();

	GetWorld()->GetTimerManager().SetTimer(SetTeammateCountHandle, this, &ThisClass::SetTeammateCount, 1.0f, true, 1.0f);
	GetWorld()->GetTimerManager().SetTimer(SetLatestEventHandle, this, &ThisClass::SetLatestEvent, 1.0f, true, 1.0f);
	GetWorld()->GetTimerManager().SetTimer(SetAveragePlayerLatencyHandle, this, &ThisClass::SetAveragePlayerLatency, 1.0f, true, 1.0f);
}

void UDayOneWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(SetTeammateCountHandle);
	GetWorld()->GetTimerManager().ClearTimer(SetLatestEventHandle);
	GetWorld()->GetTimerManager().ClearTimer(SetAveragePlayerLatencyHandle);
	Super::NativeDestruct();
}

void UDayOneWidget::SetTeammateCount()
{
	FString OwningPlayerTeam;
	APlayerState* OwningPlayerState = GetOwningPlayerState();

	if (OwningPlayerState != nullptr) {
		ADayOnePlayerState* OwningDayOnePlayerState = Cast<ADayOnePlayerState>(OwningPlayerState);
		if (OwningDayOnePlayerState != nullptr) {
			OwningPlayerTeam = OwningDayOnePlayerState->Team;
			TextBlock_TeamName->SetText(FText::FromString("Team: " + OwningPlayerTeam));
		}
	}

	if (OwningPlayerTeam.Len() > 0) {
		TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;

		int TeammateCount = 0;

		for (APlayerState* PlayerState : PlayerStates) {
			if (PlayerState != nullptr) {
				ADayOnePlayerState* DayOnePlayerState = Cast<ADayOnePlayerState>(PlayerState);
				if (DayOnePlayerState != nullptr && DayOnePlayerState->Team.Equals(OwningPlayerTeam)) {
					TeammateCount++;
				}
			}
		}

		TextBlock_TeammateCount->SetText(FText::FromString("Teammate Count: " + FString::FromInt(TeammateCount)));
	}
}

void UDayOneWidget::SetLatestEvent()
{
	FString LatestEvent;
	FString WinningTeam;
	AGameStateBase* GameState = GetWorld()->GetGameState();

	if (GameState != nullptr) {
		ADayOneGameState* DayOneGameState = Cast<ADayOneGameState>(GameState);
		if (DayOneGameState != nullptr) {
			LatestEvent = DayOneGameState->LatestEvent;
			WinningTeam = DayOneGameState->WinningTeam;
		}
	}

	if (LatestEvent.Len() > 0) {
		if (LatestEvent.Equals("GameEnded")) {
			FString OwningPlayerTeam;
			APlayerState* OwningPlayerState = GetOwningPlayerState();

			if (OwningPlayerState != nullptr) {
				ADayOnePlayerState* OwningDayOnePlayerState = Cast<ADayOnePlayerState>(OwningPlayerState);
				if (OwningDayOnePlayerState != nullptr) {
					OwningPlayerTeam = OwningDayOnePlayerState->Team;
				}
			}

			if (WinningTeam.Len() > 0 && OwningPlayerTeam.Len() > 0) {
				FString GameOverMessage = "You and the " + OwningPlayerTeam;
				if (OwningPlayerTeam.Equals(WinningTeam)) {
					TextBlock_Event->SetText(FText::FromString(GameOverMessage + " won!"));
				}
				else {
					TextBlock_Event->SetText(FText::FromString(GameOverMessage + " lost :("));
				}
			}
		}
		else {
			TextBlock_Event->SetText(FText::FromString(LatestEvent));
		}
	}
}

void UDayOneWidget::SetAveragePlayerLatency()
{
	TextBlock_Ping->SetText(FText::FromString("Latency: TODO"));
	/*
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
		if (DayOneGameInstance != nullptr) {
			float TotalPlayerLatency = 0.0f;
			for (float PlayerLatency : DayOneGameInstance->PlayerLatencies) {
				TotalPlayerLatency += PlayerLatency;
			}

			float AveragePlayerLatency = 60.0f;

			if (TotalPlayerLatency > 0) {
				AveragePlayerLatency = TotalPlayerLatency / DayOneGameInstance->PlayerLatencies.Num();

				FString PingString = "Ping: " + FString::FromInt(FMath::RoundToInt(AveragePlayerLatency)) + "ms";
				TextBlock_Ping->SetText(FText::FromString(PingString));
			}
		}
	}
	*/
}
