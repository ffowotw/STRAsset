#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"
#include "STRScriptDataFactory.generated.h"

UCLASS(hidecategories=Object)
class USTRScriptDataFactory : public UFactory
{
    GENERATED_UCLASS_BODY()

public:
    virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
};
