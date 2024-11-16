#pragma once

#include "CoreMinimal.h"
#include "STRCharaSet.h"
#include "STRCharaRenderer.generated.h"

UCLASS(BlueprintType)
class ASTRCharaRenderer : public AActor
{
    GENERATED_BODY()

public:
    ASTRCharaRenderer();

public:
    void Init(FSTRCharaSet InCharaSet);

    void SetSprite(FString InSprite);
    void Render(int InFacing, int InPositionX, int InPositionY);

private:
    UPROPERTY()
    TMap<FString, class USTRAnimSet*> m_animArray;

private:
    UPROPERTY()
    USceneComponent* m_root;
    
    UPROPERTY()
    TMap<FString, USkeletalMeshComponent*> m_meshComponents;
};
