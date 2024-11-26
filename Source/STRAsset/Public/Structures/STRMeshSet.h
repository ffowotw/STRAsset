#pragma once

#include "CoreMinimal.h"
#include "STRMeshSet.generated.h"

USTRUCT(BlueprintType)
struct FSTRMeshSet
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TArray<FString> Meshes;
};
