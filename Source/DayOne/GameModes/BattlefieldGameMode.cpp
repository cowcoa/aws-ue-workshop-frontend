// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlefieldGameMode.h"

#include "DayOne/Characters/DayOneCharacter.h"

void ABattlefieldGameMode::PlayerEliminated(ADayOneCharacter* ElimmedCharacter,
                                            ADayOnePlayerController* VictimController, ADayOnePlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}
}
