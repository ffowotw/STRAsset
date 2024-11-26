#pragma once

#include "CoreMinimal.h"
#include "DataAssets/STRSoundCueArray.h"
#include "DataAssets/STRCameraAnimDataAsset.h"
#include "Engine/DataAsset.h"
#include "Structures/STRDataSet.h"
#include "STRDataArray.generated.h"

UCLASS(BlueprintType)
class USTRDataArray : public UDataAsset
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, CallInEditor)
    void LoadData()
    {
        #if WITH_EDITOR

        DataSet.Empty();

        for (FString chara : CharaList)
        {
            class USTRScriptData* script = Load<USTRScriptData>("/Game/Chara/" + chara + "/Common/Data/SCR_" + chara);
            class USTRScriptData* effectScript = Load<USTRScriptData>("/Game/Chara/" + chara + "/Common/Data/SCR_" + chara + "_EF");
            class USTRCollisionData* collision = Load<USTRCollisionData>("/Game/Chara/" + chara + "/Common/Data/COL_" + chara);

            class USTRSoundCueArray* se = Load<USTRSoundCueArray>("/Game/Chara/" + chara + "/Common/Audio/SE/SEData");
            class USTRSoundCueArray* voice = Load<USTRSoundCueArray>("/Game/Chara/" + chara + "/Common/Audio/Voice/VoiceData");

            class USTRCameraAnimDataAsset* camera = Load<USTRCameraAnimDataAsset>("/Game/Chara/" + chara + "/Common/Camera/CameraData");

            class USTRParticleDataAsset* particle = Load<USTRParticleDataAsset>("/Game/Chara/" + chara + "/Common/Effect/Particles/ParticleData");
            
            // UE_LOG(LogTemp, Warning, TEXT("%s"), *GetPathName());

            DataSet.Add(chara, {
                script,
                effectScript,
                collision,
                se,
                voice,
                camera,
                particle,
            });
        }

        #endif
    }

    bool TryGet(FString InKey, FSTRDataSet& OutDataSet)
    {
        if (DataSet.Contains(InKey))
        {
            OutDataSet = DataSet[InKey];
            
            return true;
        }

        return false;
    }

public:
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> CharaList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FSTRDataSet> DataSet;

private:
    template<typename T>
    T* Load(FString InPath)
    {
        return LoadObject<T>(NULL, *InPath, NULL, LOAD_None, NULL);
    }
};
