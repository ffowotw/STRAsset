#include "STRMeshArrayFactoryNew.h"
#include "STRMeshArray.h"

USTRMeshArrayFactoryNew::USTRMeshArrayFactoryNew(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = USTRMeshArray::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USTRMeshArrayFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<USTRMeshArray>(InParent, InClass, InName, Flags);
}

bool USTRMeshArrayFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
