// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	FTimerHandle SetAveragePlayerLatencyHandle;

	UPROPERTY()
	FTimerHandle PollMatchmakingHandle;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UWebBrowser* WebBrowser_Login;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Matchmaking;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_Wins;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_Losses;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_Ping;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_MatchmakingEvent;

private:
	FHttpModule* HttpModule;

	UPROPERTY()
	FString LoginUrl;

	UPROPERTY()
	FString ApiUrl;

	UPROPERTY()
	FString CallbackUrl;

	UPROPERTY()
	FString RegionCode;

	UPROPERTY()
	float AveragePlayerLatency;

	UPROPERTY()
	bool SearchingForGame;

	UFUNCTION()
	void HandleLoginUrlChange();

	UFUNCTION()
	void SetAveragePlayerLatency();

	UFUNCTION()
	void OnMatchmakingButtonClicked();

	UFUNCTION()
	void PollMatchmaking();

	void OnExchangeCodeForTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnGetPlayerDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnStartMatchmakingResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnStopMatchmakingResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnPollMatchmakingResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
