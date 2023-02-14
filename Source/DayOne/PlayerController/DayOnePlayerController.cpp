// Fill out your copyright notice in the Description page of Project Settings.


#include "DayOnePlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "DayOne/Characters/DayOneCharacter.h"
#include "DayOne/Components/CombatComponent.h"
#include "DayOne/GameModes/BattlefieldGameMode.h"
#include "DayOne/HUD/CombatHUD.h"
#include "DayOne/UI/AnnouncementWidget.h"
#include "DayOne/UI/CharacterOverlayWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

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
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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

void ADayOnePlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	CombatHUD = CombatHUD == nullptr ? Cast<ACombatHUD>(GetHUD()) : CombatHUD;
	bool bHUDValid = CombatHUD &&
		CombatHUD->Announcement &&
		CombatHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			CombatHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		CombatHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ADayOnePlayerController::BeginPlay()
{
	Super::BeginPlay();

	CombatHUD = Cast<ACombatHUD>(GetHUD());
	ServerCheckMatchState();
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

void ADayOnePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MatchState);
}

void ADayOnePlayerController::SetHUDTime()
{
	/*
	float TimeLeft = MatchTime - GetServerTime();
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(TimeLeft);
	}

	CountdownInt = SecondsLeft;
	*/
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	if (HasAuthority())
	{
		if (BattlefieldGameMode == nullptr)
		{
			BattlefieldGameMode = Cast<ABattlefieldGameMode>(UGameplayStatics::GetGameMode(this));
			LevelStartingTime = BattlefieldGameMode->LevelStartingTime;
		}
		BattlefieldGameMode = BattlefieldGameMode == nullptr ? Cast<ABattlefieldGameMode>(UGameplayStatics::GetGameMode(this)) : BattlefieldGameMode;
		if (BattlefieldGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BattlefieldGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
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
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
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

void ADayOnePlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ADayOnePlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	CombatHUD = CombatHUD == nullptr ? Cast<ACombatHUD>(GetHUD()) : CombatHUD;
	if (CombatHUD)
	{
		if (CombatHUD->CharacterOverlay == nullptr) CombatHUD->AddCharacterOverlay();
		if (CombatHUD->Announcement)
		{
			CombatHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ADayOnePlayerController::HandleCooldown()
{
	CombatHUD = CombatHUD == nullptr ? Cast<ACombatHUD>(GetHUD()) : CombatHUD;
	if (CombatHUD)
	{
		CombatHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = CombatHUD->Announcement && 
			CombatHUD->Announcement->AnnouncementText && 
			CombatHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			CombatHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			CombatHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			CombatHUD->Announcement->InfoText->SetText(FText());
		}
	}
	ADayOneCharacter* DayOneCharacter = Cast<ADayOneCharacter>(GetPawn());
	if (DayOneCharacter && DayOneCharacter->GetCombat())
	{
		DayOneCharacter->bDisableGameplay = true;
		DayOneCharacter->GetCombat()->FireButtonPressed(false);
	}
}

void ADayOnePlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ADayOnePlayerController::ServerCheckMatchState_Implementation()
{
	ABattlefieldGameMode* GameMode = Cast<ABattlefieldGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void ADayOnePlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match,
	float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (CombatHUD && MatchState == MatchState::WaitingToStart)
	{
		CombatHUD->AddAnnouncement();
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
