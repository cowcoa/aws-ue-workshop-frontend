// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
	FTimerHandle UpdateLatencyUIHandle;

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
	class FGameLiftClientModule* GLClientModule;

	UPROPERTY()
	bool bSearchingForGameSession;

	UFUNCTION()
	void UpdateLatencyUI();

	UFUNCTION()
	void OnMatchmakingButtonClicked();

	UFUNCTION()
	void PollMatchmaking();

	void OnGLLoginResponse(FString AuthzCode);
	void OnGLExchangeCodeToTokensResponse(FString AccessToken, FString RefreshToken, int ExpiresIn);
	void OnGLGetPlayerDataResponse(FString PlayerId, int Wins, int Losses);
	void OnGLStartMatchmakingResponse(FString TicketId);
	void OnGLStopMatchmakingResponse();
	void OnGLPollMatchmakingResponse(FString TicketType, FString PlayerId, FString PlayerSessionId, FString IpAddress, FString Port);
};
