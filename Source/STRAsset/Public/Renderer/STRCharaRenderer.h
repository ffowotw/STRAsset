#pragma once

#include "CoreMinimal.h"
#include "STRDataSet.h"
#include "Renderer/STRObjectRenderer.h"
#include "STRCharaRenderer.generated.h"

UCLASS(BlueprintType)
class ASTRCharaRenderer : public ASTRObjectRenderer
{
    GENERATED_BODY()

public:
    virtual void Init(FSTRDataSet InDataSet) override;

    virtual void SetSprite(FString InSprite) override;

    void UpdateMeshSet(TArray<FString> InMeshSet, TArray<FString> InHiddenMeshes);

private:
    UPROPERTY()
    TMap<FString, class USTRAnimSet*> m_animArray;

private:
    UPROPERTY()
    TMap<FString, USkeletalMeshComponent*> m_meshComponents;
};
