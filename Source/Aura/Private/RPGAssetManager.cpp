// Copyright Kyle Murray


#include "RPGAssetManager.h"
#include "RPGGameplayTags.h"

URPGAssetManager& URPGAssetManager::Get()
{
	// TODO: insert return statement here
	checkf(GEngine, TEXT("GEngine not initialized, please initialize GEngine"));

	URPGAssetManager* RPGAssetManager = Cast<URPGAssetManager>(GEngine->AssetManager);
	return *RPGAssetManager;
}

void URPGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FRPGGameplayTags::InitializeNativeGameplayTags();
}
