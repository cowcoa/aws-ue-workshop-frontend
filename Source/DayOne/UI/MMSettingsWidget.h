// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MMSettingsWidget.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UMMSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMMSettingsWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	FTimerHandle PollMatchmakingHandle;
	UFUNCTION()
	void PollMatchmaking();
	
	FTimerHandle UpdateLatencyUIHandle;
	UFUNCTION()
	void UpdateLatencyUI();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UWebBrowser* WebBrowser_Login;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_Latency;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_PlayerId;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_Score;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_MatchmakingEvent;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TestButton_1;
	UFUNCTION()
	void OnTestButton_1Clicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TestButton_2;
	UFUNCTION()
	void OnTestButton_2Clicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TestButton_3;
	UFUNCTION()
	void OnTestButton_3Clicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackButton;
	UFUNCTION()
	void OnBackButton_Clicked();

	//
	UPROPERTY()
	TSubclassOf<UUserWidget> LoginWidgetClass;

	UPROPERTY()
	UUserWidget* LoginWidget;

	class FGameLiftClientModule* GLClientModule;

	bool bSearchingForGameSession;

	void OnGLLoginResponse(FString AuthzCode);
	void OnGLExchangeCodeToTokensResponse(FString AccessToken, FString RefreshToken, int ExpiresIn);
	void OnGLGetPlayerDataResponse(FString PlayerId, int Wins, int Losses);
	void OnGLStartMatchmakingResponse(FString TicketId);
	void OnGLStopMatchmakingResponse();
	void OnGLPollMatchmakingResponse(FString TicketType, FString PlayerId, FString PlayerSessionId, FString IpAddress, FString Port);
};
