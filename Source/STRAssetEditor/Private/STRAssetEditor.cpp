// Copyright Epic Games, Inc. All Rights Reserved.

#include "STRAssetEditor.h"
#include "IAssetTools.h"
#include "STRTRScriptDataActions.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FSTRAssetEditorModule"

void FSTRAssetEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type assetType = assetTools.RegisterAdvancedAssetCategory(FName(TEXT("STR Asset")), LOCTEXT("STRAsset", "STR Asset"));

	m_trscriptDataAction = MakeShareable(new FSTRTRScriptDataActions(assetType));
	m_collisionDataAction = MakeShareable(new FSTRCollisionDataActions(assetType));
	m_meshArrayAction = MakeShareable(new FSTRMeshArrayActions(assetType));
	m_animSetAction = MakeShareable(new FSTRAnimSetActions(assetType));
	m_animArrayAction = MakeShareable(new FSTRAnimArrayActions(assetType));

	assetTools.RegisterAssetTypeActions(m_trscriptDataAction.ToSharedRef());
	assetTools.RegisterAssetTypeActions(m_collisionDataAction.ToSharedRef());
	assetTools.RegisterAssetTypeActions(m_meshArrayAction.ToSharedRef());
	assetTools.RegisterAssetTypeActions(m_animSetAction.ToSharedRef());
	assetTools.RegisterAssetTypeActions(m_animArrayAction.ToSharedRef());
}

void FSTRAssetEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		return;
	}

	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(m_trscriptDataAction.ToSharedRef());
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(m_collisionDataAction.ToSharedRef());
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(m_meshArrayAction.ToSharedRef());
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(m_animSetAction.ToSharedRef());
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(m_animArrayAction.ToSharedRef());
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSTRAssetEditorModule, STRAssetEditor)
