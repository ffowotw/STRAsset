#include "STRGameMode.h"
#include "STRPawn.h"
#include "Camera/STRCamera.h"
#include "DataAssets/STRDataArray.h"
#include "Object/STRChara.h"
#include "Object/STREffectObject.h"
#include "Object/STRObject.h"
#include "Particle/STRParticle.h"
#include "Renderer/STRCharaRenderer.h"
#include "Renderer/STRObjectRenderer.h"
#include "DrawDebugHelpers.h"

void ASTRGameMode::BeginPlay()
{
    FActorSpawnParameters spawnInfo;

    m_camera = GetWorld()->SpawnActor<ASTRCamera>(ASTRCamera::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, spawnInfo);

    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bCanEverTick = true;

    int32 charaIndex;
    
    USTRChara* chara = CreateChara(DebugP2, "P2", charaIndex);

    if (chara)
    {
        chara->SetPosition(252000, 0);
    }
}

void ASTRGameMode::Tick(float InDeltaTime)
{
    DrawDebugLine(GetWorld(), FVector(-1515000, 0, 0) / float(1000), FVector(-1515000, 0, 1000000) / float(1000), FColor::Red, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(1515000, 0, 0) / float(1000), FVector(1515000, 0, 1000000) / float(1000), FColor::Red, false, -1, 0, 3);
    
    ObjectTicking();
    CameraTicking();
}

void ASTRGameMode::ObjectTicking()
{
    for (USTRChara* chara : m_charaList)
    {
        chara->TickFacing();
        chara->InputTicking();
    }

    for (USTRObject* obj : GetTickableObjectList())
    {
        obj->EarlyTicking();
        obj->Ticking();
        obj->LateTicking();
    }

    for (USTRChara* chara : m_charaList)
    {
        chara->TickPushboxCheck();
    }

    for (USTRObject* obj : m_objectList)
    {
        obj->TickHitCheck();
    }

    for (USTRObject* obj : m_objectList)
    {
        obj->TickDamageCheck();
    }

    for (int32 i = m_objectList.Num() - 1; i >= 0; i--)
    {
        if (m_objectList[i]->CheckRequestedDestroy())
        {
            m_objectList.RemoveAt(i);
        }
    }
    
    for (USTRObject* obj : m_objectList)
    {
        obj->DecreaseFreezeTime();
        obj->TickDrawCollisions();
        obj->TickRender();
    }
}

void ASTRGameMode::CameraTicking()
{
    if (m_charaList.Num() == 2)
    {
        int32 x, y, minX, maxX, minY, maxY;
        bool minXSet, maxXSet, minYSet, maxYSet;

        minX = 0;
        maxX = 0;
        minY = 0;
        maxY = 0;
        minXSet = false;
        maxXSet = false;
        minYSet = false;
        maxYSet = false;

        for (USTRChara* chara : m_charaList)
        {
            chara->GetPosition(x, y);

            if (!minXSet || x < minX)
            {
                minXSet = true;

                minX = x;
            }
            
            if (!maxXSet || x > maxX)
            {
                maxXSet = true;

                maxX = x;
            }

            if (!minYSet || y < minY)
            {
                minYSet = true;

                minY = y;
            }
            else if (!maxYSet || y > maxY)
            {
                maxYSet = true;

                maxY = y;
            }
        }

        int32 offsetX, offsetY, offsetZ;
        int32 offsetY_X, offsetY_Y;

        offsetX = FMath::Clamp((minX + maxX) / 2, -1000000, 1000000);
        offsetY_X = FMath::Clamp(maxX - minX - 800000, 0, 250000);
        offsetY_Y = FMath::Clamp(maxY - minY - 200000, 0, 250000);
        offsetY = FMath::Max(offsetY_X, offsetY_Y);
        offsetZ = FMath::Max(minY + FMath::Max(maxY - 200000, 0) + maxY - 300000, 0) / 2;

        if (offsetY < m_cameraLastY)
        {
            if (!m_cameraZoomInEnabled && maxY == minY)
            {
                m_cameraZoomInEnabled = true;
            }

            if (m_cameraZoomInDelayTimer == 0)
            {
                m_cameraZoomInDelayTimer = 20;
            }
            else if (m_cameraZoomInEnabled)
            {
                m_cameraZoomInDelayTimer--;
            }

            if (m_cameraZoomInDelayTimer > 0)
            {
                offsetY = m_cameraLastY;
            }
            else
            {
                m_cameraZoomInDelayTimer = -1;

                m_cameraLastY = offsetY;
            }
        }
        else
        {
            m_cameraZoomInDelayTimer = 0;
            m_cameraZoomInEnabled = false;

            m_cameraLastY = offsetY;
        }

        FSTRTransform transform = {
            offsetX,
            // 540000,
            // 106423,
            1250000 + offsetY,
            (1250000 + offsetY) / 5 + offsetZ,
            // 1500000,
            // 300000,
            450,
            -16200,
            0
        };

        m_camera->SetCameraTransform(transform);
        m_camera->Ticking();

        // DrawDebugLine(GetWorld(), FVector(offsetX, 0, 0) / float(1000), FVector(offsetX, 0, 1000000) / float(1000), FColor::Red, false, -1, 0, 3);
    }
}

int32 ASTRGameMode::AssignPlayer(ASTRPawn* playerPawn)
{
    int32 playerIndex = m_playerList.Num();
    int32 charaIndex;

    UE_LOG(LogTemp, Warning, TEXT("Player Assigned at: %i"), playerIndex);

    USTRChara* chara = CreateChara(DebugP1, "P1", charaIndex);

    if (charaIndex == -1)
    {
        return -1;
    }

    chara->SetPosition(-252000, 0);

    m_playerList.Add(playerPawn);
    m_playerControllingChara.Emplace(playerIndex, charaIndex);

    APlayerController* controller = Cast<APlayerController>(playerPawn->GetController());

    controller->SetViewTarget(m_camera);

    return playerIndex;
}

void ASTRGameMode::PlayerInput(int32 InIndex, TArray<FKey> InKeyMappings, FKey InKey, bool InPressed)
{
    if (InIndex == -1 || !m_playerControllingChara.Contains(InIndex) || !InKeyMappings.Contains(InKey))
    {
        return;
    }

    int32 charaIndex = m_playerControllingChara[InIndex];
    int32 buttonIndex;

    if (charaIndex >= m_charaList.Num() || charaIndex < 0)
    {
        return;
    }

    InKeyMappings.Find(InKey, buttonIndex);

    switch(buttonIndex)
    {
        case 0:
        {
            m_charaList[charaIndex]->PlayerUp(InPressed);
            
            break;
        }
        case 1:
        {
            m_charaList[charaIndex]->PlayerDown(InPressed);
            
            break;
        }
        case 2:
        {
            m_charaList[charaIndex]->PlayerRight(InPressed);
            
            break;
        }
        case 3:
        {
            m_charaList[charaIndex]->PlayerLeft(InPressed);
            
            break;
        }
        default:
        {
            int32 buttonCount = GetPlayerButtons().Num();
            uint8 button = FMath::Max(buttonIndex - 4, 0);

            if (button < 0 || button > buttonCount)
            {
                break;
            }

            m_charaList[charaIndex]->PlayerButton(button, InPressed);

            break;
        }
    }
}

void ASTRGameMode::CreateObject(USTRObject* InParent, FString InObjectName, FSTRDataSet InDataSet)
{
    if (!InDataSet.EffectScriptData || !InDataSet.CollisionData)
    {
        return;
    }

    if (!InDataSet.EffectScriptData->Subroutines.Contains(InObjectName))
    {
        return;
    }

    USTREffectObject* object = NewObject<USTREffectObject>();
    
    object->Init(this, InParent, InObjectName, InDataSet);

    m_objectList.Add(object);
}

USTRChara* ASTRGameMode::CreateChara(FString InCharaName, FString InCharaLayer, int32& OutCharaIndex)
{
    FSTRDataSet dataSet;
    if (!DataArray->TryGet(InCharaName, dataSet))
    {
        OutCharaIndex = -1;

        return nullptr;
    }

    USTRChara* chara = NewObject<USTRChara>();
    int32 charaIndex = m_charaList.Num();

    FActorSpawnParameters spawnInfo;

    ASTRCharaRenderer* renderer = GetWorld()->SpawnActor<ASTRCharaRenderer>(ASTRCharaRenderer::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, spawnInfo);

    chara->PreInit(this, charaIndex, InCharaLayer, renderer, dataSet);
    chara->Init();

    m_objectList.Add(chara);
    m_charaList.Add(chara);

    OutCharaIndex = charaIndex;

    return chara;
}

ASTRParticle* ASTRGameMode::CreateParticle(class USTRParticleDataAsset* InParticleDataAsset, FString InParticleName)
{
    if (InParticleDataAsset && InParticleDataAsset->Particles.Contains(InParticleName))
    {
        FActorSpawnParameters spawnInfo;

        ASTRParticle* particle = GetWorld()->SpawnActor<ASTRParticle>(ASTRParticle::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, spawnInfo);

        particle->Init(InParticleDataAsset->Particles[InParticleName]);

        return particle;
    }

    return nullptr;
}

TArray<USTRChara*> ASTRGameMode::GetOpponentCharaList(FString InLayer, bool InContainsIgnore)
{
    TArray<USTRChara*> result;

    for(USTRChara* chara : m_charaList)
    {
        FString charaLayer = chara->GetLayer();

        if (InLayer == "P1" || InLayer == "P2")
        {
            if (charaLayer != InLayer)
            {
                result.Add(chara);
            }
        }
        else
        {
            bool ignore = (!InContainsIgnore && charaLayer != "IGNORE") || InContainsIgnore;

            if (ignore && charaLayer != InLayer)
            {
                result.Add(chara);
            }
        }

    }

    return result;
}

void ASTRGameMode::DrawCollision(FSTRCollision InCollision, FColor InColor)
{
    int32 x = InCollision.X;
    int32 y = InCollision.Y;
    int32 w = InCollision.Width;
    int32 h = InCollision.Height;

    DrawDebugLine(GetWorld(), FVector(x - 5000, 0, y - 5000) / float(1000), FVector(x - 5000, 0, y + 5000) / float(1000), FColor::Black, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(x - 5000, 0, y + 5000) / float(1000), FVector(x + 5000, 0, y + 5000) / float(1000), FColor::Black, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(x + 5000, 0, y + 5000) / float(1000), FVector(x + 5000, 0, y - 5000) / float(1000), FColor::Black, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(x + 5000, 0, y - 5000) / float(1000), FVector(x - 5000, 0, y - 5000) / float(1000), FColor::Black, false, -1, 0, 3);

    DrawDebugLine(GetWorld(), FVector(x, 0, y) / float(1000), FVector(x, 0, y + h) / float(1000), InColor, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(x, 0, y + h) / float(1000), FVector(x + w, 0, y + h) / float(1000), InColor, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(x + w, 0, y + h) / float(1000), FVector(x + w, 0, y) / float(1000), InColor, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(x + w, 0, y) / float(1000), FVector(x, 0, y) / float(1000), InColor, false, -1, 0, 3);
}
