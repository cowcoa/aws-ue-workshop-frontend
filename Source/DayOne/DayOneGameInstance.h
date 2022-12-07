// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "DayOneGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UDayOneGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDayOneGameInstance();

	UPROPERTY()
	FString AccessToken;

	UPROPERTY()
	FString IdToken;

	UPROPERTY()
	FString RefreshToken;

	UPROPERTY()
	FString MatchmakingTicketId;

	UPROPERTY()
	FTimerHandle RetrieveNewTokensHandle;

	UPROPERTY()
	FTimerHandle GetResponseTimeHandle;

	TDoubleLinkedList<float> PlayerLatencies;

	UFUNCTION()
	void SetCognitoTokens(FString NewAccessToken, FString NewIdToken, FString NewRefreshToken);

protected:
	virtual void Init() override;
	virtual void Shutdown() override;

private:
	FHttpModule* HttpModule;

	UPROPERTY()
	FString ApiUrl;

	UPROPERTY()
	FString RegionCode;

	UFUNCTION()
	void RetrieveNewTokens();

	UFUNCTION()
	void GetResponseTime();

	void OnRetrieveNewTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnGetResponseTimeResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
