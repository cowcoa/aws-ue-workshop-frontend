// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"

#include "GameLiftClientModule.h"
#include "WebBrowser.h"
#include "IWebBrowserCookieManager.h"
#include "WebBrowserModule.h"
#include "IWebBrowserSingleton.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "DayOne/DayOneGameInstance.h"
#include "Kismet/GameplayStatics.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GLClientModule = &FGameLiftClientModule::Get();
	bSearchingForGameSession = false;
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bIsFocusable = true;

	// Register handler to process matchmaking button clicked.
	Button_Matchmaking->OnClicked.AddUniqueDynamic(this, &ThisClass::OnMatchmakingButtonClicked);
	// Setup timer to update latency UI text
	UE_LOG(LogTemp, Warning, TEXT("SetTimer->UpdateLatencyUI"));
	GetWorld()->GetTimerManager().SetTimer(UpdateLatencyUIHandle, this, &UMainMenuWidget::UpdateLatencyUI, 1.0f, true, 1.0f);

	// Check if Cognito token is valid.
	// If token still valid we don't need to show login Ui to player. Just update player info to display.
	if (GLClientModule->GameLiftClient->IsTokenValid())
	{
		GLClientModule->GameLiftClient->GetPlayerData().BindUObject(this, &ThisClass::OnGLGetPlayerDataResponse);
	}
	else
	{
		// Clean the cookies.
		IWebBrowserSingleton* WebBrowserSingleton = IWebBrowserModule::Get().GetSingleton();
		if (WebBrowserSingleton != nullptr) {
			TOptional<FString> DefaultContext;
			TSharedPtr<IWebBrowserCookieManager> CookieManager = WebBrowserSingleton->GetCookieManager(DefaultContext);
			if (CookieManager.IsValid()) {
				CookieManager->DeleteCookies();
			}
		}

		// Show Cognito Hosted login UI to player.
		GLClientModule->GameLiftClient->ShowLoginUI(*WebBrowser_Login).AddUObject(this, &ThisClass::OnGLLoginResponse);
	}
}

void UMainMenuWidget::NativeDestruct()
{
	if (bSearchingForGameSession)
	{
		GLClientModule->GameLiftClient->StopMatchmaking();
	}
	
	GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingHandle);
	GetWorld()->GetTimerManager().ClearTimer(UpdateLatencyUIHandle);
	Super::NativeDestruct();
}

void UMainMenuWidget::UpdateLatencyUI()
{
	FString PingString("Latency\n");
	for (const TPair<FString, float>& Pair : GLClientModule->GameLiftClient->AverageLatencyPerRegion)
	{
		FString RegionPingString = FString::Printf(TEXT("%s: %dms\n"), *Pair.Key,  FMath::RoundToInt(Pair.Value));
		PingString += RegionPingString;
	}
	TextBlock_Ping->SetText(FText::FromString(PingString));
}

void UMainMenuWidget::OnMatchmakingButtonClicked()
{
	// Disable button to prevent player click it multi times.
	Button_Matchmaking->SetIsEnabled(false);

	// If we are already in searching process, just cancel it.
	if (bSearchingForGameSession)
	{
		bSearchingForGameSession = false;
		GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingHandle);
		
		GLClientModule->GameLiftClient->StopMatchmaking().BindUObject(this, &ThisClass::OnGLStopMatchmakingResponse);
	}
	else // We are not in searching, start matchmaking
	{
		GLClientModule->GameLiftClient->StartMatchmaking().BindUObject(this, &ThisClass::OnGLStartMatchmakingResponse);
	}
}

void UMainMenuWidget::PollMatchmaking()
{
	if (bSearchingForGameSession)
	{
		GLClientModule->GameLiftClient->PollMatchmaking().BindUObject(this, &ThisClass::OnGLPollMatchmakingResponse);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingHandle);
	}
}

// --------------------------------------

void UMainMenuWidget::OnGLLoginResponse(FString AuthzCode)
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnGLLoginResponse AuthzCode: %s"), *AuthzCode);

	GLClientModule->GameLiftClient->ExchangeCodeToTokens(AuthzCode).AddUObject(this, &ThisClass::OnGLExchangeCodeToTokensResponse);
}

void UMainMenuWidget::OnGLExchangeCodeToTokensResponse(FString AccessToken, FString RefreshToken, int ExpiresIn)
{
	UE_LOG(LogTemp, Warning, TEXT("AccessToken: %s"), *AccessToken);
	UE_LOG(LogTemp, Warning, TEXT("RefreshToken: %s"), *RefreshToken);
	UE_LOG(LogTemp, Warning, TEXT("ExpiresIn: %d"), ExpiresIn);

	// just for FIRE refresh token timer
	// TODO: change SetCognitoTokens code.
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
		if (DayOneGameInstance != nullptr) {
			DayOneGameInstance->SetRefreshTokensTimer();
		}
	}

	GLClientModule->GameLiftClient->GetPlayerData().BindUObject(this, &ThisClass::OnGLGetPlayerDataResponse);
}

void UMainMenuWidget::OnGLGetPlayerDataResponse(FString PlayerId, int Wins, int Losses)
{
	UE_LOG(LogTemp, Warning, TEXT("PlayerId: %s, Wins: %d, Losses: %d"), *PlayerId, Wins, Losses);

	TextBlock_Wins->SetText(FText::FromString(FString::Printf(TEXT("Wins: %d"), Wins)));
	TextBlock_Losses->SetText(FText::FromString(FString::Printf(TEXT("Losses: %d"), Losses)));

	WebBrowser_Login->SetVisibility(ESlateVisibility::Hidden);
	Button_Matchmaking->SetVisibility(ESlateVisibility::Visible);
	TextBlock_Wins->SetVisibility(ESlateVisibility::Visible);
	TextBlock_Losses->SetVisibility(ESlateVisibility::Visible);
	TextBlock_Ping->SetVisibility(ESlateVisibility::Visible);
	TextBlock_MatchmakingEvent->SetVisibility(ESlateVisibility::Visible);

	/*
	LatencyMap Latency;
	Latency.Add("ap-northeast-1", 70.0f);
	GLClientModule->GameLiftClient->StartMatchmaking(Latency).BindUObject(this, &ThisClass::OnGLStartMatchmakingResponse);
	*/
}

void UMainMenuWidget::OnGLStartMatchmakingResponse(FString TicketId)
{
	UE_LOG(LogTemp, Warning, TEXT("TicketId: %s"), *TicketId);

	bSearchingForGameSession = true;
	UE_LOG(LogTemp, Warning, TEXT("SetTimer->PollMatchmakingHandle"));
	GetWorld()->GetTimerManager().SetTimer(PollMatchmakingHandle, this, &UMainMenuWidget::PollMatchmaking, 10.0f, true, 3.0f);
	
	UTextBlock* ButtonTextBlock = (UTextBlock*)Button_Matchmaking->GetChildAt(0);
	ButtonTextBlock->SetText(FText::FromString("Cancel Matchmaking"));
	TextBlock_MatchmakingEvent->SetText(FText::FromString("Currently looking for a match"));

	Button_Matchmaking->SetIsEnabled(true);
	/*
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ThisClass::PollGLMatchmaking, TicketId);
	GetWorld()->GetTimerManager().SetTimer(PollMatchmakingTimerHandle, TimerDelegate, 5.0f, true, 1.0f);
	*/
}

void UMainMenuWidget::OnGLStopMatchmakingResponse()
{
	UTextBlock* ButtonTextBlock = (UTextBlock*)Button_Matchmaking->GetChildAt(0);
	ButtonTextBlock->SetText(FText::FromString("Join Game"));
	TextBlock_MatchmakingEvent->SetText(FText::FromString(""));

	Button_Matchmaking->SetIsEnabled(true);
}

void UMainMenuWidget::OnGLPollMatchmakingResponse(FString TicketType, FString PlayerId, FString PlayerSessionId,
	FString IpAddress, FString Port)
{
	UE_LOG(LogTemp, Warning, TEXT("TicketType: %s, PlayerId: %s, PlayerSessionId: %s, IpAddress: %s, Port: %s"), *TicketType, *PlayerId, *PlayerSessionId, *IpAddress, *Port);

	if (bSearchingForGameSession)
	{
		GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingHandle);
		bSearchingForGameSession = false;

		if (TicketType.Equals("MatchmakingSucceeded"))
		{
			Button_Matchmaking->SetIsEnabled(false);
			TextBlock_MatchmakingEvent->SetText(FText::FromString("Successfully found a match. Now connecting to the server..."));

			FString LevelName = IpAddress + ":" + Port;
			const FString& Options = "?PlayerSessionId=" + PlayerSessionId + "?PlayerId=" + PlayerId;
			UE_LOG(LogTemp, Warning, TEXT("options: %s"), *Options);

			UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName), false, Options);
		}
		else
		{
			UTextBlock* ButtonTextBlock = (UTextBlock*) Button_Matchmaking->GetChildAt(0);
			ButtonTextBlock->SetText(FText::FromString("Join Game"));
			TextBlock_MatchmakingEvent->SetText(FText::FromString(TicketType + ". Please try again"));
		}
	}
}
