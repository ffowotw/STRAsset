#include "Renderer/STRCharaRenderer.h"
#include "Anim/STRAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "DataAssets/STRAnimArray.h"
#include "DataAssets/STRAnimSet.h"
#include "DataAssets/STRMeshArray.h"

void ASTRCharaRenderer::Init(FSTRDataSet InDataSet)
{
    m_animArray.Empty();

    for (USTRAnimArray* animArray : InDataSet.AnimArrays)
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

    for (const TPair<FString, USkeletalMesh*>& pair : InDataSet.MeshArray->Meshes)
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

void ASTRCharaRenderer::UpdateMeshSet(TArray<FString> InMeshSet, TArray<FString> InHiddenMeshes)
{
    for (const TPair<FString, USkeletalMeshComponent*>& pair : m_meshComponents)
    {
        pair.Value->SetVisibility(InMeshSet.Contains(pair.Key) && !InHiddenMeshes.Contains(pair.Key));
        // if (!InMeshSet.Contains(pair.Key) || InHiddenMeshes.Contains(pair.Key))
        // {
            
        // }
        // else
        // {
        //     //
        // }
    }
}
