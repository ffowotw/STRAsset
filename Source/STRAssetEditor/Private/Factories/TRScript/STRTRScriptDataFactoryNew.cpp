#include "STRTRScriptDataFactoryNew.h"
#include "STRTRScriptData.h"

USTRTRScriptDataFactoryNew::USTRTRScriptDataFactoryNew(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = USTRTRScriptData::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USTRTRScriptDataFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<USTRTRScriptData>(InParent, InClass, InName, Flags);
}

bool USTRTRScriptDataFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
