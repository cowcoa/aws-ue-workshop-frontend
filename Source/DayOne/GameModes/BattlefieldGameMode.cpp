// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlefieldGameMode.h"

#include "DayOne/Characters/DayOneCharacter.h"
#include "DayOne/PlayerController/DayOnePlayerController.h"
#include "DayOne/PlayerStates/DayOnePlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

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
