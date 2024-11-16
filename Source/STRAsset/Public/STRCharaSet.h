#pragma once

#include "CoreMinimal.h"
#include "STRCharaSet.generated.h"

USTRUCT(BlueprintType)
struct FSTRCharaSet
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRTRScriptData* ScriptData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRCollisionData* CollisionData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRMeshArray* MeshArray;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class USTRAnimArray*> AnimArrays;
};
