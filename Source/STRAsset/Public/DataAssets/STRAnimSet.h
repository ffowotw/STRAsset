#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "STRAnimSet.generated.h"

UCLASS(BlueprintType)
class USTRAnimSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UAnimBlueprint* AnimBP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class UAnimSequence*> AnimSet;
};
