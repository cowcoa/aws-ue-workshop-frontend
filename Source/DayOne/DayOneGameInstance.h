// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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
	FTimerHandle RefreshTokensHandle;

	UPROPERTY()
	FTimerHandle TestLatencyHandle;

	UFUNCTION()
	void SetRefreshTokensTimer();

protected:
	virtual void Init() override;
	virtual void Shutdown() override;

private:
	class FGameLiftClientModule* GLClientModule;

	UFUNCTION()
	void RefreshTokens();

	UFUNCTION()
	void TestLatency();
	
	//
	void OnGLRefreshTokensResponse(FString AccessToken, bool bIsSuccessful);
};
