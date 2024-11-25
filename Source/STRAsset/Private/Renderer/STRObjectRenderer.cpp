#include "Renderer/STRObjectRenderer.h"

ASTRObjectRenderer::ASTRObjectRenderer()
{
    m_root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = m_root;
}

void ASTRObjectRenderer::Init(FSTRDataSet InDataSet)
{
    m_dataSet = InDataSet;
}

void ASTRObjectRenderer::SetSprite(FString InSprite)
{
    TArray<FString> spriteValues;

    InSprite.ParseIntoArray(spriteValues, TEXT("_"), true);

    if (spriteValues.Num() != 2)
    {
        return;
    }
    
    //
}

void ASTRObjectRenderer::Render(int32 InFacing, int32 InPositionX, int32 InPositionY)
{
    SetActorScale3D(FVector(InFacing, 1, 1));
    SetActorLocation(FVector(float(InPositionX) / 1000, 0, float(InPositionY) / 1000));
}
