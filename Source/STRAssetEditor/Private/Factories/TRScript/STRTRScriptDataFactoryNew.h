#pragma once

#include "CoreMinimal.h"
#include "STRTRScriptDataFactoryNew.generated.h"

UCLASS(hidecategories=Object)
class USTRTRScriptDataFactoryNew : public UFactory
{
    GENERATED_UCLASS_BODY()

public:
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
    virtual bool ShouldShowInNewMenu() const override;
};
