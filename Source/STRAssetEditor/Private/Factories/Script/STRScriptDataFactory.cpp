#include "STRScriptDataFactory.h"
#include "STRScriptData.h"
#include "Misc/FileHelper.h"

USTRScriptDataFactory::USTRScriptDataFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Formats.Add(FString(TEXT("trscript;TR Script")));
    SupportedClass = USTRScriptData::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
}

UObject* USTRScriptDataFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
    USTRScriptData* scriptData = nullptr;
	FString textString;
    
    if (FFileHelper::LoadFileToString(textString, *Filename))
    {
        scriptData = NewObject<USTRScriptData>(InParent, InClass, InName, Flags);

        scriptData->SetScriptText(textString);
        scriptData->GenerateSubroutines();
    }

    bOutOperationCanceled = false;

    return scriptData;
}
