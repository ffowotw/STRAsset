#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "STRSoundCueArray.generated.h"

UCLASS(BlueprintType)
class USTRSoundCueArray : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class USoundCue*> SoundCues;
};
