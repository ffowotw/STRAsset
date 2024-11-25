#pragma once

#include "CoreMinimal.h"
#include "STRDataSet.generated.h"

USTRUCT(BlueprintType)
struct FSTRDataSet
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRScriptData* ScriptData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRScriptData* EffectScriptData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRCollisionData* CollisionData;
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRMeshArray* MeshArray;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class USTRAnimArray*> AnimArrays;
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRParticleDataAsset* ParticleDataAsset;
};
