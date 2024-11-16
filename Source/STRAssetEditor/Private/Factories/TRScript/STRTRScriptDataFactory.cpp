#include "STRTRScriptDataFactory.h"
#include "STRTRScriptData.h"
#include "Misc/FileHelper.h"

USTRTRScriptDataFactory::USTRTRScriptDataFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    Formats.Add(FString(TEXT("trscript;TR Script")));
    SupportedClass = USTRTRScriptData::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
}

UObject* USTRTRScriptDataFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
    USTRTRScriptData* scriptData = nullptr;
	FString textString;
    
    if (FFileHelper::LoadFileToString(textString, *Filename))
    {
        scriptData = NewObject<USTRTRScriptData>(InParent, InClass, InName, Flags);
        
        scriptData->SetScriptText(textString);
        scriptData->GenerateSubroutines();
    }

    bOutOperationCanceled = false;

    return scriptData;
}
