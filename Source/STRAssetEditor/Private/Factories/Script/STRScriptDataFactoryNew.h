#pragma once

#include "CoreMinimal.h"
#include "STRScriptDataFactoryNew.generated.h"

UCLASS(hidecategories=Object)
class USTRScriptDataFactoryNew : public UFactory
{
    GENERATED_UCLASS_BODY()

public:
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
    virtual bool ShouldShowInNewMenu() const override;
};
