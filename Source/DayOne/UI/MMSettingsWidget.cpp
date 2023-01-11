// Fill out your copyright notice in the Description page of Project Settings.

#include "MMSettingsWidget.h"
#include "GameLiftClientModule.h"
#include "Components/Button.h"

UMMSettingsWidget::UMMSettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> LoginWidgetObject(TEXT("/Game/DayOne/UI/WBP_MainMenuWidget"));
	LoginWidgetClass = LoginWidgetObject.Class;

	GLClientModule = &FGameLiftClientModule::Get();
}

void UMMSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TestButton_1)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnTestButton_1Clicked registered ---"));
		TestButton_1->OnClicked.AddDynamic(this, &ThisClass::OnTestButton_1Clicked);
		if (GLClientModule->GameLiftClient->IsTokenValid())
		{
			TestButton_1->SetIsEnabled(false);
		}
	}
	if (TestButton_2)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnTestButton_2Clicked registered ---"));
		TestButton_2->OnClicked.AddDynamic(this, &ThisClass::OnTestButton_2Clicked);
	}
	if (TestButton_3)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnTestButton_3Clicked registered ---"));
		TestButton_3->OnClicked.AddDynamic(this, &ThisClass::OnTestButton_3Clicked);
	}
	if (BackButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnBackButton_Clicked registered ---"));
		BackButton->OnClicked.AddDynamic(this, &ThisClass::OnBackButton_Clicked);
	}
}

void UMMSettingsWidget::NativeDestruct()
{
	if (LoginWidget != nullptr && LoginWidget->IsInViewport())
	{
		LoginWidget->RemoveFromParent();
	}
	
	Super::NativeDestruct();
}

void UMMSettingsWidget::OnTestButton_1Clicked()
{
	TestButton_1->SetIsEnabled(false);
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Blue,
		FString::Printf(TEXT("OnTestButton_1Clicked"))
	);

	if (LoginWidget == nullptr)
	{
		LoginWidget = CreateWidget<UUserWidget>(GetWorld(), LoginWidgetClass);
	}
	LoginWidget->AddToViewport();

	GLClientModule->GameLiftClient->OnExchangeCodeToTokensResponse.AddUObject(this, &ThisClass::OnGLExchangeCodeToTokensResponse);
}

void UMMSettingsWidget::OnTestButton_2Clicked()
{
	GEngine->AddOnScreenDebugMessage(
	-1,
	15.f,
	FColor::Blue,
	FString::Printf(TEXT("OnTestButton_2Clicked"))
);
}

void UMMSettingsWidget::OnTestButton_3Clicked()
{
	GEngine->AddOnScreenDebugMessage(
	-1,
	15.f,
	FColor::Blue,
	FString::Printf(TEXT("OnTestButton_3Clicked"))
);
}

void UMMSettingsWidget::OnBackButton_Clicked()
{

}

void UMMSettingsWidget::OnGLExchangeCodeToTokensResponse(FString AccessToken, FString RefreshToken, int ExpiresIn)
{
	UE_LOG(LogTemp, Warning, TEXT("ExchangeCodeToTokensResponse broadcast to MM widget"));
	TestButton_2->SetIsEnabled(true);
}
