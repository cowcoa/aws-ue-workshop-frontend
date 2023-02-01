// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DisplayText;
};
