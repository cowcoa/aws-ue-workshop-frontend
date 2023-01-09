#include "GameLiftClientModule.h"

IMPLEMENT_MODULE(FGameLiftClientModule, GameLiftClient);

FGameLiftClientModule* FGameLiftClientModule::Singleton = NULL;

void FGameLiftClientModule::StartupModule()
{
	Singleton = this;
	GameLiftClient = NewObject<UGameLiftClient>();
	GameLiftClient->AddToRoot();
}

void FGameLiftClientModule::ShutdownModule()
{
	GameLiftClient->RemoveFromRoot();
	Singleton = nullptr;
}

FGameLiftClientModule& FGameLiftClientModule::Get()
{
	if (Singleton == NULL)
	{
		check(IsInGameThread());
		FModuleManager::LoadModuleChecked<FGameLiftClientModule>("GameLiftClient");
	}
	check(Singleton != NULL);
	return *Singleton;
}
