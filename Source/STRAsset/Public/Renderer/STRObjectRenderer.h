#pragma once

#include "CoreMinimal.h"
#include "STRDataSet.h"
#include "STRObjectRenderer.generated.h"

UCLASS(BlueprintType)
class ASTRObjectRenderer : public AActor
{
    GENERATED_BODY()

public:
    ASTRObjectRenderer();

public:
    virtual void Init(FSTRDataSet InDataSet);

    virtual void SetSprite(FString InSprite);
    
    void Render(int32 InFacing, int32 InPositionX, int32 InPositionY);

protected:
    class USTRParticleDataAsset* GetParticleDataAsset() { return m_dataSet.ParticleDataAsset; }

protected:
    UPROPERTY()
    FSTRDataSet m_dataSet;

    UPROPERTY()
    USceneComponent* m_root;
};
