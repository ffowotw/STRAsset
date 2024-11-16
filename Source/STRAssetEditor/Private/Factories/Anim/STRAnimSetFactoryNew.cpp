#include "STRAnimSetFactoryNew.h"
#include "Anim/STRAnimSet.h"

USTRAnimSetFactoryNew::USTRAnimSetFactoryNew(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = USTRAnimSet::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USTRAnimSetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<USTRAnimSet>(InParent, InClass, InName, Flags);
}

bool USTRAnimSetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
