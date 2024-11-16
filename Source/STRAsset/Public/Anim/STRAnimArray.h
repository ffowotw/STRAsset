#pragma once

#include "CoreMinimal.h"
#include "STRAnimArray.generated.h"

UCLASS(BlueprintType)
class STRASSET_API USTRAnimArray : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class USTRAnimSet*> AnimArray;
};
