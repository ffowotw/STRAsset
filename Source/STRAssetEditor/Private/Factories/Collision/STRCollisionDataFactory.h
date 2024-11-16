#pragma once

#include "CoreMinimal.h"
#include "STRCollisionDataFactory.generated.h"

UCLASS(hidecategories=Object)
class USTRCollisionDataFactory : public UFactory
{
    GENERATED_UCLASS_BODY()

public:
    virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
};
