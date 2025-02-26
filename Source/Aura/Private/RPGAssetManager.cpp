// Copyright Kyle Murray


#include "RPGAssetManager.h"
#include "RPGGameplayTags.h"
#include "AbilitySystemGlobals.h"

URPGAssetManager& URPGAssetManager::Get()
{
	checkf(GEngine, TEXT("GEngine not initialized, please initialize GEngine"));

	URPGAssetManager* RPGAssetManager = Cast<URPGAssetManager>(GEngine->AssetManager);
	return *RPGAssetManager;
}

void URPGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FRPGGameplayTags::InitializeNativeGameplayTags();


	//THIS IS REQUIRED TO USE TARGET DATA
	UAbilitySystemGlobals::Get().InitGlobalData();
}
