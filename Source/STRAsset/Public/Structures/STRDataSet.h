#pragma once

#include "CoreMinimal.h"
#include "Structures/STRCostume.h"
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
    class USTRSoundCueArray* SEDataAsset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRSoundCueArray* VoiceDataAsset;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRCameraAnimDataAsset* CameraAnimDataAsset;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class USTRParticleDataAsset* ParticleDataAsset;
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSTRCostume> Costumes;
};
