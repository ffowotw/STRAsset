#include "STRCollisionDataFactory.h"
#include "STRCollisionData.h"
#include "Misc/FileHelper.h"

USTRCollisionDataFactory::USTRCollisionDataFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Formats.Add(FString(TEXT("collision;Collision")));
    SupportedClass = USTRCollisionData::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
}

UObject* USTRCollisionDataFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
    USTRCollisionData* scriptData = nullptr;
	FString textString;
    
    if (FFileHelper::LoadFileToString(textString, *Filename))
    {
        scriptData = NewObject<USTRCollisionData>(InParent, InClass, InName, Flags);
        
        scriptData->SetScriptText(textString);
        scriptData->GenerateCollisions();
    }

    bOutOperationCanceled = false;

    return scriptData;
}
