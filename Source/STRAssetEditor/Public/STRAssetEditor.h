// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "STRTRScriptDataActions.h"
#include "STRCollisionDataActions.h"
#include "STRMeshArrayActions.h"
#include "STRAnimSetActions.h"
#include "STRAnimArrayActions.h"

class FSTRAssetEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FSTRTRScriptDataActions> m_trscriptDataAction;
	TSharedPtr<FSTRCollisionDataActions> m_collisionDataAction;
	TSharedPtr<FSTRMeshArrayActions> m_meshArrayAction;
	TSharedPtr<FSTRAnimSetActions> m_animSetAction;
	TSharedPtr<FSTRAnimArrayActions> m_animArrayAction;
};
