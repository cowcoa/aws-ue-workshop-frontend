// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DayOneWidget.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UDayOneWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FTimerHandle SetTeammateCountHandle;

	UPROPERTY()
	FTimerHandle SetLatestEventHandle;

	UPROPERTY()
	FTimerHandle SetAveragePlayerLatencyHandle;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_TeamName;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_TeammateCount;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_Event;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TextBlock_Ping;
	
private:
	class FGameLiftClientModule* GLClientModule;
	
	UFUNCTION()
	void SetTeammateCount();

	UFUNCTION()
	void SetLatestEvent();

	UFUNCTION()
	void SetAveragePlayerLatency();
};
