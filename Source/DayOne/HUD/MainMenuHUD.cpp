// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuHUD.h"
#include "Blueprint/UserWidget.h"

AMainMenuHUD::AMainMenuHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MainMenuWidgetObject(TEXT("/Game/DayOne/UI/WBP_MainMenuWidget"));
	MainMenuWidgetClass = MainMenuWidgetObject.Class;
}

void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController != nullptr) {
		PlayerController->bShowMouseCursor = true;
	}

	if (MainMenuWidgetClass != nullptr) {
		UUserWidget* MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
		if (MainMenuWidget != nullptr) {
			MainMenuWidget->AddToViewport();
			MainMenuWidget->SetFocus();
		}
	}
}
