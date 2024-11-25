#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "STRParticleDataAsset.generated.h"

UCLASS(BlueprintType)
class USTRParticleDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class UNiagaraSystem*> Particles;
};
