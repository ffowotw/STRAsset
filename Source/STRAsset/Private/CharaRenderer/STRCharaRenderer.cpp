#include "CharaRenderer/STRCharaRenderer.h"
#include "STRMeshArray.h"
#include "Anim/STRAnimArray.h"
#include "Anim/STRAnimSet.h"
#include "Anim/STRAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

ASTRCharaRenderer::ASTRCharaRenderer()
{
    m_root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = m_root;
}

void ASTRCharaRenderer::Init(FSTRCharaSet InCharaSet)
{
    m_animArray.Empty();

    for (USTRAnimArray* animArray : InCharaSet.AnimArrays)
    {
        for (const TPair<FString, USTRAnimSet*>& arrayPair : animArray->AnimArray)
        {
            if (!m_animArray.Contains(arrayPair.Key))
            {
                m_animArray.Add(arrayPair.Key, arrayPair.Value);
            }
            else
            {
                m_animArray[arrayPair.Key]->AnimSet.Append(arrayPair.Value->AnimSet);
            }
        }
    }

    for (const TPair<FString, USkeletalMesh*>& pair : InCharaSet.MeshArray->Meshes)
    {
        // UE_LOG(LogTemp, Warning, TEXT("%s"), *pair.Key);

        USkeletalMeshComponent* mesh = NewObject<USkeletalMeshComponent>(this, *pair.Key);
        
        mesh->SetupAttachment(m_root);
        mesh->SetSkeletalMesh(pair.Value);
        mesh->SetAnimInstanceClass(m_animArray[pair.Key]->AnimBP->GetAnimBlueprintGeneratedClass());
        mesh->RegisterComponent();

        m_meshComponents.Add(pair.Key, mesh);
    }
}

void ASTRCharaRenderer::SetSprite(FString InSprite)
{
    TArray<FString> spriteValues;

    InSprite.ParseIntoArray(spriteValues, TEXT("_"), true);

    if (spriteValues.Num() != 2)
    {
        return;
    }

    for (const TPair<FString, USkeletalMeshComponent*>& pair : m_meshComponents)
    {
        USTRAnimInstance* animInstance = Cast<USTRAnimInstance>(pair.Value->GetAnimInstance());

        if (!animInstance)
        {
            continue;
        }

        if (m_animArray.Contains(pair.Key) && m_animArray[pair.Key]->AnimSet.Contains(spriteValues[0]))
        {
            animInstance->SetAnimAndFrame(m_animArray[pair.Key]->AnimSet[spriteValues[0]], FCString::Atoi(*spriteValues[1]));
        }
    }
}

void ASTRCharaRenderer::Render(int InFacing, int InPositionX, int InPositionY)
{
    SetActorScale3D(FVector(InFacing, 1, 1));
    SetActorLocation(FVector(float(InPositionX) / 1000, 0, float(InPositionY) / 1000));
}
