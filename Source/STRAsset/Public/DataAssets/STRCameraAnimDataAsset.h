#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "STRCameraAnimDataAsset.generated.h"

UCLASS(BlueprintType)
class USTRCameraAnimDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class UCameraAnim*> CameraAnimations;
};
