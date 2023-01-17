// Fill out your copyright notice in the Description page of Project Settings.


#include "DayOneGameInstance.h"
#include "GameLiftClientModule.h"

UDayOneGameInstance::UDayOneGameInstance()
{
	GLClientModule = &FGameLiftClientModule::Get();
}

void UDayOneGameInstance::SetRefreshTokensTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("SetTimer->RefreshTokens in SetRefreshTokensTimer"));
	GetWorld()->GetTimerManager().SetTimer(RefreshTokensHandle, this, &UDayOneGameInstance::RefreshTokens, 1.0f, false, GLClientModule->GameLiftClient->TokenExpiresIn - 300);
}

void UDayOneGameInstance::Init()
{
	Super::Init();

#if !WITH_GAMELIFT
	UE_LOG(LogTemp, Warning, TEXT("SetTimer->TestLatency"));
	GetWorld()->GetTimerManager().SetTimer(TestLatencyHandle, this, &UDayOneGameInstance::TestLatency, 1.0f, true, 1.0f);
#endif
}

void UDayOneGameInstance::Shutdown()
{
	GetWorld()->GetTimerManager().ClearTimer(RefreshTokensHandle);
	GetWorld()->GetTimerManager().ClearTimer(TestLatencyHandle);

#if !WITH_GAMELIFT
	GLClientModule->GameLiftClient->RevokeTokens();
#endif
	
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
		UE_LOG(LogTemp, Warning, TEXT("SetTimer->RefreshTokens in OnGLRefreshTokensResponse"));
		GetWorld()->GetTimerManager().SetTimer(RefreshTokensHandle, this, &UDayOneGameInstance::RefreshTokens, 1.0f, false, 30.0f);
	}
}
