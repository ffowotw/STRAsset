#pragma once

#include "CoreMinimal.h"
#include "STRCostume.generated.h"

USTRUCT(BlueprintType)
struct FSTRCostume
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRMeshArray* MeshArray;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class USTRAnimArray*> AnimArrays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class USTRMaterialSet*> Colors;
};
