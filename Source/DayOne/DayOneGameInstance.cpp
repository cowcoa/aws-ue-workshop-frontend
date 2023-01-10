// Fill out your copyright notice in the Description page of Project Settings.


#include "DayOneGameInstance.h"
#include "GameLiftClientModule.h"

UDayOneGameInstance::UDayOneGameInstance()
{
	GLClientModule = &FGameLiftClientModule::Get();
}

void UDayOneGameInstance::SetRefreshTokensTimer()
{
	GetWorld()->GetTimerManager().SetTimer(RefreshTokensHandle, this, &UDayOneGameInstance::RefreshTokens, 1.0f, false, GLClientModule->GameLiftClient->TokenExpiresIn - 300);
}

void UDayOneGameInstance::Init()
{
	Super::Init();

	GetWorld()->GetTimerManager().SetTimer(TestLatencyHandle, this, &UDayOneGameInstance::TestLatency, 1.0f, true, 1.0f);
}

void UDayOneGameInstance::Shutdown()
{
	GetWorld()->GetTimerManager().ClearTimer(RefreshTokensHandle);
	GetWorld()->GetTimerManager().ClearTimer(TestLatencyHandle);
	
	GLClientModule->GameLiftClient->RevokeTokens();
	
	Super::Shutdown();
}

void UDayOneGameInstance::RefreshTokens()
{
	GLClientModule->GameLiftClient->RefreshTokens().BindUObject(this, &ThisClass::OnGLRefreshTokensResponse);
}

void UDayOneGameInstance::TestLatency()
{
	GLClientModule->GameLiftClient->TestLatency();
}

void UDayOneGameInstance::OnGLRefreshTokensResponse(FString AccessToken, bool bIsSuccessful)
{
	if (bIsSuccessful)
	{
		// Reset normal timer
		SetRefreshTokensTimer();
	}
	else
	{
		// Set faster timer
		GetWorld()->GetTimerManager().SetTimer(RefreshTokensHandle, this, &UDayOneGameInstance::RefreshTokens, 1.0f, false, 30.0f);
	}
}
