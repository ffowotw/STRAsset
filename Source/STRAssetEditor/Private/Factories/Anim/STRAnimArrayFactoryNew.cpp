#include "STRAnimArrayFactoryNew.h"
#include "Anim/STRAnimArray.h"

USTRAnimArrayFactoryNew::USTRAnimArrayFactoryNew(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = USTRAnimArray::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USTRAnimArrayFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<USTRAnimArray>(InParent, InClass, InName, Flags);
}

bool USTRAnimArrayFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
