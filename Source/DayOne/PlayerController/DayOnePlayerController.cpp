// Fill out your copyright notice in the Description page of Project Settings.


#include "DayOnePlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "DayOne/HUD/CombatHUD.h"
#include "DayOne/UI/CharacterOverlayWidget.h"

void ADayOnePlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	CombatHUD = CombatHUD == nullptr ? Cast<ACombatHUD>(GetHUD()) : CombatHUD;
	bool bHUDValid = CombatHUD &&
		CombatHUD->CharacterOverlay &&
		CombatHUD->CharacterOverlay->HealthBar &&
		CombatHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		CombatHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		CombatHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ADayOnePlayerController::BeginPlay()
{
	Super::BeginPlay();

	CombatHUD = Cast<ACombatHUD>(GetHUD());
}

void ADayOnePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}
