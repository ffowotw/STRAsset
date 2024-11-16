#pragma once

#include "CoreMinimal.h"
#include "STRAnimSet.generated.h"

UCLASS(BlueprintType)
class STRASSET_API USTRAnimSet : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UAnimBlueprint* AnimBP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class UAnimSequence*> AnimSet;
};
