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
#include "DayOne/Components/TextFileReaderComponent.h"
#include "Kismet/GameplayStatics.h"

UMainMenuWidget::UMainMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UTextFileReaderComponent* TextFileReader = CreateDefaultSubobject<UTextFileReaderComponent>(TEXT("UTextFileReader"));
	LoginUrl = TextFileReader->ReadFile("DayOne/Data/LoginUrl.txt");
	ApiUrl = TextFileReader->ReadFile("DayOne/Data/ApiUrl.txt");
	CallbackUrl = TextFileReader->ReadFile("DayOne/Data/CallbackUrl.txt");
	RegionCode = TextFileReader->ReadFile("DayOne/Data/RegionCode.txt");

	HttpModule = &FHttpModule::Get();
	GLClientModule = &FGameLiftClientModule::Get();

	AveragePlayerLatency = 60.0;
	SearchingForGame = false;
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
	
	Button_Matchmaking->OnClicked.AddUniqueDynamic(this, &ThisClass::OnMatchmakingButtonClicked);

	GetWorld()->GetTimerManager().SetTimer(SetAveragePlayerLatencyHandle, this, &UMainMenuWidget::SetAveragePlayerLatency, 1.0f, true, 1.0f);

	/*
	FString AccessToken;
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
		if (DayOneGameInstance != nullptr) {
			AccessToken = DayOneGameInstance->AccessToken;
		}
	}
	*/

	// Check if token is valid.
	if (GLClientModule->GameLiftClient->IsTokenValid())
	{
		/*
		TSharedRef<IHttpRequest> GetPlayerDataRequest = HttpModule->CreateRequest();
		GetPlayerDataRequest->OnProcessRequestComplete().BindUObject(this, &UMainMenuWidget::OnGetPlayerDataResponseReceived);
		GetPlayerDataRequest->SetURL(ApiUrl + "/getplayerdata");
		GetPlayerDataRequest->SetVerb("GET");
		//GetPlayerDataRequest->SetHeader("Content-Type", "application/json");
		GetPlayerDataRequest->SetHeader("Authorization", AccessToken);
		GetPlayerDataRequest->ProcessRequest();
		*/
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

		// Load Cognito Hosted UI.
		//WebBrowser_Login->LoadURL(LoginUrl);
		//WebBrowser_Login->OnUrlChanged.AddUniqueDynamic(this, &ThisClass::OnLoginUrlChanged);
		GLClientModule->GameLiftClient->ShowCowLoginUI(*WebBrowser_Login).BindUObject(this, &ThisClass::OnGLLoginResponse);

		//GLClientModule->GameLiftClient->ExchangeCodeToTokens("24b65619-0cdf-44c7-91b8-5b62426be2ec").BindUObject(this, &ThisClass::OnGLExchangeCodeToTokensResponse);
	}
}

void UMainMenuWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingHandle);
	GetWorld()->GetTimerManager().ClearTimer(SetAveragePlayerLatencyHandle);
	Super::NativeDestruct();
}

void UMainMenuWidget::OnLoginUrlChanged(const FText& BrowserUrl)
{
	FString Url;
	FString QueryParameters;

	UE_LOG(LogTemp, Warning, TEXT("BrowserUrl: %s"), *BrowserUrl.ToString())

	// Only process Cognito Hosted UI login callback URL.
	if (BrowserUrl.ToString().Split("?", &Url, &QueryParameters)) {
		if (Url.Equals(CallbackUrl)) {
			FString ParameterName;
			FString ParameterValue;

			// Extract Cognito login code.
			if (QueryParameters.Split("=", &ParameterName, &ParameterValue)) {
				if (ParameterName.Equals("code")) {
					// Remove the end character '#'
					if (ParameterValue.EndsWith("#"))
					{
						ParameterValue.MidInline(0, ParameterValue.Len() - 1);
					}

					TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
					RequestObj->SetStringField(ParameterName, ParameterValue);

					FString RequestBody;
					TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);

					if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
						TSharedRef<IHttpRequest> ExchangeCodeForTokensRequest = HttpModule->CreateRequest();
						ExchangeCodeForTokensRequest->OnProcessRequestComplete().BindUObject(this, &UMainMenuWidget::OnExchangeCodeForTokensResponseReceived);
						ExchangeCodeForTokensRequest->SetURL(ApiUrl + "/exchangecodefortokens");
						ExchangeCodeForTokensRequest->SetVerb("POST");
						ExchangeCodeForTokensRequest->SetHeader("Content-Type", "application/json");
						ExchangeCodeForTokensRequest->SetContentAsString(RequestBody);
						ExchangeCodeForTokensRequest->ProcessRequest();
					}
				}
			}
		}
	}
}

void UMainMenuWidget::SetAveragePlayerLatency()
{
	for (const TPair<FString, float>& Pair : GLClientModule->GameLiftClient->AverageLatencyPerRegion)
	{
		FString PingString = FString::Printf(TEXT("%s Ping: %d\n"), *Pair.Key,  FMath::RoundToInt(Pair.Value));
		//FString PingString = "Ping: " + FString::FromInt(FMath::RoundToInt(AveragePlayerLatency)) + "ms";
		TextBlock_Ping->SetText(FText::FromString(PingString));
	}

	/*
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
		if (DayOneGameInstance != nullptr)
		{
			float TotalPlayerLatency = 0.0f;
			for (float PlayerLatency : DayOneGameInstance->PlayerLatencies) {
				TotalPlayerLatency += PlayerLatency;
			}

			if (TotalPlayerLatency > 0) {
				AveragePlayerLatency = TotalPlayerLatency / DayOneGameInstance->PlayerLatencies.Num();

				FString PingString = "Ping: " + FString::FromInt(FMath::RoundToInt(AveragePlayerLatency)) + "ms";
				TextBlock_Ping->SetText(FText::FromString(PingString));
			}
		}
	}
	*/
}

void UMainMenuWidget::OnMatchmakingButtonClicked()
{
	Button_Matchmaking->SetIsEnabled(false);

	/*
	FString AccessToken;
	FString MatchmakingTicketId;
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
		if (DayOneGameInstance != nullptr) {
			AccessToken = DayOneGameInstance->AccessToken;
			MatchmakingTicketId = DayOneGameInstance->MatchmakingTicketId;
		}
	}
	*/

	if (SearchingForGame)
	{
		GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingHandle);
		SearchingForGame = false;
		
		if (GLClientModule->GameLiftClient->IsTokenValid() && GLClientModule->GameLiftClient->TicketId.Len() > 0)
		{
			GLClientModule->GameLiftClient->StopMatchmaking().BindUObject(this, &ThisClass::OnGLStopMatchmakingResponse);
			/*
			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetStringField("ticketId", MatchmakingTicketId);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest> StopMatchmakingRequest = HttpModule->CreateRequest();
				StopMatchmakingRequest->OnProcessRequestComplete().BindUObject(this, &UMainMenuWidget::OnStopMatchmakingResponseReceived);
				StopMatchmakingRequest->SetURL(ApiUrl + "/stopmatchmaking");
				StopMatchmakingRequest->SetVerb("POST");
				StopMatchmakingRequest->SetHeader("Content-Type", "application/json");
				StopMatchmakingRequest->SetHeader("Authorization", AccessToken);
				StopMatchmakingRequest->SetContentAsString(RequestBody);
				StopMatchmakingRequest->ProcessRequest();
			}
			else {
				UTextBlock* ButtonTextBlock = (UTextBlock*)Button_Matchmaking->GetChildAt(0);
				ButtonTextBlock->SetText(FText::FromString("Join Game"));
				TextBlock_MatchmakingEvent->SetText(FText::FromString(""));

				Button_Matchmaking->SetIsEnabled(true);
			}
			*/
		}
		else {
			UTextBlock* ButtonTextBlock = (UTextBlock*)Button_Matchmaking->GetChildAt(0);
			ButtonTextBlock->SetText(FText::FromString("Join Game"));
			TextBlock_MatchmakingEvent->SetText(FText::FromString(""));

			Button_Matchmaking->SetIsEnabled(true);
		}
	}
	else {
		if (GLClientModule->GameLiftClient->IsTokenValid())
		{
			GLClientModule->GameLiftClient->StartMatchmaking().BindUObject(this, &ThisClass::OnGLStartMatchmakingResponse);
			
			/*
			TSharedRef<FJsonObject> LatencyMapObj = MakeShareable(new FJsonObject);
			LatencyMapObj->SetNumberField(RegionCode, AveragePlayerLatency);

			TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
			RequestObj->SetObjectField("latencyMap", LatencyMapObj);

			FString RequestBody;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
			if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
				TSharedRef<IHttpRequest> StartMatchmakingRequest = HttpModule->CreateRequest();
				StartMatchmakingRequest->OnProcessRequestComplete().BindUObject(this, &UMainMenuWidget::OnStartMatchmakingResponseReceived);
				StartMatchmakingRequest->SetURL(ApiUrl + "/startmatchmaking");
				StartMatchmakingRequest->SetVerb("POST");
				StartMatchmakingRequest->SetHeader("Content-Type", "application/json");
				StartMatchmakingRequest->SetHeader("Authorization", AccessToken);
				StartMatchmakingRequest->SetContentAsString(RequestBody);
				StartMatchmakingRequest->ProcessRequest();
			}
			else {
				Button_Matchmaking->SetIsEnabled(true);
			}
			*/
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cognito tokens are invalid"));
			Button_Matchmaking->SetIsEnabled(true);
		}
	}
}

void UMainMenuWidget::PollMatchmaking()
{
	/*
	FString AccessToken;
	FString MatchmakingTicketId;

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
		if (DayOneGameInstance != nullptr) {
			AccessToken = DayOneGameInstance->AccessToken;
			MatchmakingTicketId = DayOneGameInstance->MatchmakingTicketId;
		}
	}

	if (AccessToken.Len() > 0 && MatchmakingTicketId.Len() > 0 && SearchingForGame) {
		TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject);
		RequestObj->SetStringField("ticketId", MatchmakingTicketId);

		FString RequestBody;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
		if (FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer)) {
			TSharedRef<IHttpRequest> PollMatchmakingRequest = HttpModule->CreateRequest();
			PollMatchmakingRequest->OnProcessRequestComplete().BindUObject(this, &UMainMenuWidget::OnPollMatchmakingResponseReceived);
			PollMatchmakingRequest->SetURL(ApiUrl + "/pollmatchmaking");
			PollMatchmakingRequest->SetVerb("POST");
			PollMatchmakingRequest->SetHeader("Content-Type", "application/json");
			PollMatchmakingRequest->SetHeader("Authorization", AccessToken);
			PollMatchmakingRequest->SetContentAsString(RequestBody);
			PollMatchmakingRequest->ProcessRequest();
		}
	}
	*/

	if (GLClientModule->GameLiftClient->IsTokenValid() && SearchingForGame)
	{
		GLClientModule->GameLiftClient->PollMatchmaking().BindUObject(this, &ThisClass::OnGLPollMatchmakingResponse);
	}
}

void UMainMenuWidget::OnExchangeCodeForTokensResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                              bool bWasSuccessful)
{
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("access_token") && JsonObject->HasField("id_token") && JsonObject->HasField("refresh_token")) {
				UGameInstance* GameInstance = GetGameInstance();
				if (GameInstance != nullptr) {
					UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
					if (DayOneGameInstance != nullptr) {
						FString AccessToken = JsonObject->GetStringField("access_token");
						FString IdToken = JsonObject->GetStringField("id_token");
						FString RefreshToken = JsonObject->GetStringField("refresh_token");
						DayOneGameInstance->SetCognitoTokens(AccessToken, IdToken, RefreshToken);

						TSharedRef<IHttpRequest> GetPlayerDataRequest = HttpModule->CreateRequest();
						GetPlayerDataRequest->OnProcessRequestComplete().BindUObject(this, &UMainMenuWidget::OnGetPlayerDataResponseReceived);
						GetPlayerDataRequest->SetURL(ApiUrl + "/getplayerdata");
						GetPlayerDataRequest->SetVerb("GET");
						GetPlayerDataRequest->SetHeader("Content-Type", "application/json");
						GetPlayerDataRequest->SetHeader("Authorization", AccessToken);
						GetPlayerDataRequest->ProcessRequest();
					}
				}
			}
		}
	}
}

void UMainMenuWidget::OnGetPlayerDataResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("playerData")) {
				TSharedPtr<FJsonObject> PlayerData = JsonObject->GetObjectField("playerData");
				TSharedPtr<FJsonObject> WinsObject = PlayerData->GetObjectField("Wins");
				TSharedPtr<FJsonObject> LossesObject = PlayerData->GetObjectField("Losses");

				FString Wins = WinsObject->GetStringField("N");
				FString Losses = LossesObject->GetStringField("N");

				TextBlock_Wins->SetText(FText::FromString("Wins: " + Wins));
				TextBlock_Losses->SetText(FText::FromString("Losses: " + Losses));

				WebBrowser_Login->SetVisibility(ESlateVisibility::Hidden);
				Button_Matchmaking->SetVisibility(ESlateVisibility::Visible);
				TextBlock_Wins->SetVisibility(ESlateVisibility::Visible);
				TextBlock_Losses->SetVisibility(ESlateVisibility::Visible);
				TextBlock_Ping->SetVisibility(ESlateVisibility::Visible);
				TextBlock_MatchmakingEvent->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void UMainMenuWidget::OnStartMatchmakingResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (bWasSuccessful) {
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("ticketId")) {
				FString MatchmakingTicketId = JsonObject->GetStringField("ticketId");

				UGameInstance* GameInstance = GetGameInstance();
				if (GameInstance != nullptr) {
					UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
					if (DayOneGameInstance != nullptr) {
						DayOneGameInstance->MatchmakingTicketId = MatchmakingTicketId;

						GetWorld()->GetTimerManager().SetTimer(PollMatchmakingHandle, this, &UMainMenuWidget::PollMatchmaking, 1.0f, true, 1.0f);
						SearchingForGame = true;

						UTextBlock* ButtonTextBlock = (UTextBlock*)Button_Matchmaking->GetChildAt(0);
						ButtonTextBlock->SetText(FText::FromString("Cancel Matchmaking"));
						TextBlock_MatchmakingEvent->SetText(FText::FromString("Currently looking for a match"));
					}
				}
			}
		}
	}
	Button_Matchmaking->SetIsEnabled(true);
}

void UMainMenuWidget::OnStopMatchmakingResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
		if (DayOneGameInstance != nullptr) {
			DayOneGameInstance->MatchmakingTicketId = "";
		}
	}

	UTextBlock* ButtonTextBlock = (UTextBlock*)Button_Matchmaking->GetChildAt(0);
	ButtonTextBlock->SetText(FText::FromString("Join Game"));
	TextBlock_MatchmakingEvent->SetText(FText::FromString(""));

	Button_Matchmaking->SetIsEnabled(true);
}

void UMainMenuWidget::OnPollMatchmakingResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (bWasSuccessful && SearchingForGame)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
		if (FJsonSerializer::Deserialize(Reader, JsonObject)) {
			if (JsonObject->HasField("ticket")) {
				TSharedPtr<FJsonObject> Ticket = JsonObject->GetObjectField("ticket");
				FString TicketType = Ticket->GetObjectField("Type")->GetStringField("S");

				if (TicketType.Len() > 0) {
					GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingHandle);
					SearchingForGame = false;

					UGameInstance* GameInstance = GetGameInstance();
					if (GameInstance != nullptr) {
						UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
						if (DayOneGameInstance != nullptr) {
							DayOneGameInstance->MatchmakingTicketId = "";
						}
					}

					if (TicketType.Equals("MatchmakingSucceeded")) {
						Button_Matchmaking->SetIsEnabled(false);
						TextBlock_MatchmakingEvent->SetText(FText::FromString("Successfully found a match. Now connecting to the server..."));

						TSharedPtr<FJsonObject> GameSessionInfo = Ticket->GetObjectField("GameSessionInfo")->GetObjectField("M");
						FString IpAddress = GameSessionInfo->GetObjectField("IpAddress")->GetStringField("S");
						FString Port = GameSessionInfo->GetObjectField("Port")->GetStringField("N");

						TArray<TSharedPtr<FJsonValue>> Players = Ticket->GetObjectField("Players")->GetArrayField("L");
						TSharedPtr<FJsonObject> Player = Players[0]->AsObject()->GetObjectField("M");
						FString PlayerSessionId = Player->GetObjectField("PlayerSessionId")->GetStringField("S");
						FString PlayerId = Player->GetObjectField("PlayerId")->GetStringField("S");

						FString LevelName = IpAddress + ":" + Port;
						const FString& Options = "?PlayerSessionId=" + PlayerSessionId + "?PlayerId=" + PlayerId;
						UE_LOG(LogTemp, Warning, TEXT("options: %s"), *Options);

						UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName), false, Options);
					}
					else {
						UTextBlock* ButtonTextBlock = (UTextBlock*) Button_Matchmaking->GetChildAt(0);
						ButtonTextBlock->SetText(FText::FromString("Join Game"));
						TextBlock_MatchmakingEvent->SetText(FText::FromString(TicketType + ". Please try again"));
					}
				}
			}
		}
	}
}

// --------------------------------------

void UMainMenuWidget::OnGLLoginResponse(FString AuthzCode)
{
	UE_LOG(LogTemp, Warning, TEXT("UMainMenuWidget::OnGLLoginResponse AuthzCode: %s"), *AuthzCode);

	GLClientModule->GameLiftClient->ExchangeCodeToTokens(AuthzCode).BindUObject(this, &ThisClass::OnGLExchangeCodeToTokensResponse);
}

void UMainMenuWidget::OnGLExchangeCodeToTokensResponse(FString AccessToken, FString RefreshToken, int ExpiresIn)
{
	UE_LOG(LogTemp, Warning, TEXT("AccessToken: %s"), *AccessToken);
	UE_LOG(LogTemp, Warning, TEXT("RefreshToken: %s"), *RefreshToken);
	UE_LOG(LogTemp, Warning, TEXT("ExpiresIn: %d"), ExpiresIn);

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance != nullptr) {
		UDayOneGameInstance* DayOneGameInstance = Cast<UDayOneGameInstance>(GameInstance);
		if (DayOneGameInstance != nullptr) {
			DayOneGameInstance->SetCognitoTokens(AccessToken, AccessToken, RefreshToken);
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

	GetWorld()->GetTimerManager().SetTimer(PollMatchmakingHandle, this, &UMainMenuWidget::PollMatchmaking, 10.0f, true, 3.0f);
	SearchingForGame = true;

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

void UMainMenuWidget::PollGLMatchmaking(FString TicketId)
{
	GLClientModule->GameLiftClient->PollMatchmaking().BindUObject(this, &UMainMenuWidget::OnGLPollMatchmakingResponse);
}

void UMainMenuWidget::OnGLPollMatchmakingResponse(FString TicketType, FString PlayerId, FString PlayerSessionId,
	FString IpAddress, FString Port)
{
	UE_LOG(LogTemp, Warning, TEXT("TicketType: %s, PlayerId: %s, PlayerSessionId: %s, IpAddress: %s, Port: %s"), *TicketType, *PlayerId, *PlayerSessionId, *IpAddress, *Port);

	if (SearchingForGame)
	{
		GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingHandle);
		SearchingForGame = false;

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
	
	//GetWorld()->GetTimerManager().ClearTimer(PollMatchmakingTimerHandle);
}
