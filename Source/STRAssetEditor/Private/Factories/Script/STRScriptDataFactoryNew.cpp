#include "STRScriptDataFactoryNew.h"
#include "STRScriptData.h"

USTRScriptDataFactoryNew::USTRScriptDataFactoryNew(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = USTRScriptData::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USTRScriptDataFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<USTRScriptData>(InParent, InClass, InName, Flags);
}

bool USTRScriptDataFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
