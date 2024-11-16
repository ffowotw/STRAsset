#pragma once

#include "CoreMinimal.h"
#include "STRMeshArray.generated.h"

UCLASS(BlueprintType)
class STRASSET_API USTRMeshArray : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, class USkeletalMesh*> Meshes;
};
