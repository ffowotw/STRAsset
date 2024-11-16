#include "STRCollisionDataFactoryNew.h"
#include "STRCollisionData.h"

USTRCollisionDataFactoryNew::USTRCollisionDataFactoryNew(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    SupportedClass = USTRCollisionData::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* USTRCollisionDataFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<USTRCollisionData>(InParent, InClass, InName, Flags);
}

bool USTRCollisionDataFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
