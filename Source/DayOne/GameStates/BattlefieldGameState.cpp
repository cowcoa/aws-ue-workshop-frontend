// Fill out your copyright notice in the Description page of Project Settings.


#include "BattlefieldGameState.h"

#include "Net/UnrealNetwork.h"

void ABattlefieldGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GameMessage);
	DOREPLIFETIME(ThisClass, GameOverCountDown);
}
