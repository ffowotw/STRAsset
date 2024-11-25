// Copyright Epic Games, Inc. All Rights Reserved.

#include "STRAssetEditor.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FSTRAssetEditorModule"

void FSTRAssetEditorModule::StartupModule()
{
}

void FSTRAssetEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSTRAssetEditorModule, STRAssetEditor)
