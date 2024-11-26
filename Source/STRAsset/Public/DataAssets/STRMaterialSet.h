#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "STRMaterialSet.generated.h"

UCLASS(BlueprintType)
class USTRMaterialSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class UMaterial*> Materials;
};
