// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"

#include "DayOne/HUD/MainMenuHUD.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	HUDClass = AMainMenuHUD::StaticClass();
}
