#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "STRMeshArray.generated.h"

UCLASS(BlueprintType)
class USTRMeshArray : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class USkeletalMesh*> Meshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> DefaultMeshSet;
};
