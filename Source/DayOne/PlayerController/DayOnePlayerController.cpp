// Fill out your copyright notice in the Description page of Project Settings.


#include "DayOnePlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "DayOne/Characters/DayOneCharacter.h"
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

void ADayOnePlayerController::SetHUDScore(float Score)
{
	CombatHUD = CombatHUD == nullptr ? Cast<ACombatHUD>(GetHUD()) : CombatHUD;
	bool bHUDValid = CombatHUD &&
		CombatHUD->CharacterOverlay &&
		CombatHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		CombatHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ADayOnePlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	CombatHUD = CombatHUD == nullptr ? Cast<ACombatHUD>(GetHUD()) : CombatHUD;
	bool bHUDValid = CombatHUD &&
		CombatHUD->CharacterOverlay &&
		CombatHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		CombatHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ADayOnePlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	CombatHUD = CombatHUD == nullptr ? Cast<ACombatHUD>(GetHUD()) : CombatHUD;
	bool bHUDValid = CombatHUD &&
		CombatHUD->CharacterOverlay &&
		CombatHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		CombatHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

float ADayOnePlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ADayOnePlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	CombatHUD = CombatHUD == nullptr ? Cast<ACombatHUD>(GetHUD()) : CombatHUD;
	bool bHUDValid = CombatHUD &&
		CombatHUD->CharacterOverlay &&
		CombatHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			CombatHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		CombatHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ADayOnePlayerController::BeginPlay()
{
	Super::BeginPlay();

	CombatHUD = Cast<ACombatHUD>(GetHUD());
}

void ADayOnePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void ADayOnePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ADayOneCharacter* DayOneCharacter = Cast<ADayOneCharacter>(InPawn);
	if (DayOneCharacter)
	{
		SetHUDHealth(DayOneCharacter->GetHealth(), DayOneCharacter->GetMaxHealth());
	}
}

void ADayOnePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ADayOnePlayerController::SetHUDTime()
{
	float TimeLeft = MatchTime - GetServerTime();
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(TimeLeft);
	}

	CountdownInt = SecondsLeft;
}

void ADayOnePlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (CombatHUD && CombatHUD->CharacterOverlay)
		{
			CharacterOverlay = CombatHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
			}
		}
	}
}

void ADayOnePlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ADayOnePlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ADayOnePlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
