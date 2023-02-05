// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlefieldGameMode.h"

#include "DayOne/Characters/DayOneCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ABattlefieldGameMode::PlayerEliminated(ADayOneCharacter* ElimmedCharacter,
                                            ADayOnePlayerController* VictimController, ADayOnePlayerController* AttackerController)
{
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