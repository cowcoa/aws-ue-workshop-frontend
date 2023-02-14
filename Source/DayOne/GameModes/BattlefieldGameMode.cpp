// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlefieldGameMode.h"

#include "DayOne/Characters/DayOneCharacter.h"
#include "DayOne/PlayerController/DayOnePlayerController.h"
#include "DayOne/PlayerStates/DayOnePlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABattlefieldGameMode::ABattlefieldGameMode()
{
	bDelayedStart = true;
}

void ABattlefieldGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABattlefieldGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode in WaitingToStart"))
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode in InProgress"))
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode in Cooldown"))
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABattlefieldGameMode::PlayerEliminated(ADayOneCharacter* ElimmedCharacter,
                                            ADayOnePlayerController* VictimController, ADayOnePlayerController* AttackerController)
{
	// calculate score
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ADayOnePlayerState* AttackerPlayerState = AttackerController ? Cast<ADayOnePlayerState>(AttackerController->PlayerState) : nullptr;
	ADayOnePlayerState* VictimPlayerState = VictimController ? Cast<ADayOnePlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
}

void ABattlefieldGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ABattlefieldGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ADayOnePlayerController* DayOnePlayer = Cast<ADayOnePlayerController>(*It);
		if (DayOnePlayer)
		{
			DayOnePlayer->OnMatchStateSet(MatchState);
		}
	}
}
