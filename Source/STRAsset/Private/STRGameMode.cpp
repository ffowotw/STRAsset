#include "STRGameMode.h"
#include "STRChara.h"
#include "STRPawn.h"
#include "STRCamera.h"
#include "CharaRenderer/STRCharaRenderer.h"
#include "DrawDebugHelpers.h"

void ASTRGameMode::BeginPlay()
{
    FActorSpawnParameters spawnInfo;

    m_camera = GetWorld()->SpawnActor<ASTRCamera>(ASTRCamera::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, spawnInfo);

    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bCanEverTick = true;

    int32 charaIndex;
    
    USTRChara* chara = SpawnChara(DebugP2, "P2", charaIndex);

    if (chara)
    {
        chara->SetPosition(252000, 0);
    }

    // charaMaxDistance = 1365000

    // when the distance between chara is equal or greater than 790000, zoom out
}

void ASTRGameMode::Tick(float InDeltaTime)
{
    DrawDebugLine(GetWorld(), FVector(-1515000, 0, 0) / float(1000), FVector(-1515000, 0, 1000000) / float(1000), FColor::Red, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(1515000, 0, 0) / float(1000), FVector(1515000, 0, 1000000) / float(1000), FColor::Red, false, -1, 0, 3);
    
    for (USTRChara* chara : m_charaList)
    {
        chara->TickFacing();
        chara->InputTicking();
    }

    if (m_hitStopFrame > 0)
    {
        m_hitStopFrame--;

        // UE_LOG(LogTemp, Warning, TEXT("Stop: %i"), m_hitStopFrame);
    }
    else
    {
        Ticking();
    }

    for (USTRChara* chara : m_charaList)
    {
        chara->ContinuousTicking();
        chara->TickDrawCollisions();
    }

    CameraTicking();
}

void ASTRGameMode::Ticking()
{
    int32 i;

    for (i = 0; i < m_charaList.Num(); i++)
    {
        m_charaList[i]->Ticking();
    }

    for (USTRChara* chara : m_charaList)
    {
        chara->TickPushboxCheck();
    }

    for (i = 0; i < m_charaList.Num(); i++)
    {
        m_charaList[i]->TickHitCheck();
    }

    for (i = 0; i < m_charaList.Num(); i++)
    {
        m_charaList[i]->TickDamageCheck();
    }
    
    for (USTRChara* chara : m_charaList)
    {
        chara->TickRender();
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

        offsetX = FMath::Clamp((minX + maxX) / 2, -990000, 990000);
        offsetY = FMath::Min(FMath::Max(maxX - minX - 790000, 0), 260000);
        offsetZ = FMath::Max(minY + FMath::Max(maxY - 50000, 0) + maxY - 300000, 0) / 2;

        // Z: zoom out first

        FTransformStruct transform = {
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

void ASTRGameMode::ApplyHitStop(int32 InHitStopFrame)
{
    if (InHitStopFrame > m_hitStopFrame)
    {
        m_hitStopFrame = InHitStopFrame;
    }
}

int32 ASTRGameMode::AssignPlayer(ASTRPawn* playerPawn)
{
    int32 playerIndex = m_playerList.Num();
    int32 charaIndex;

    UE_LOG(LogTemp, Warning, TEXT("Player Assigned at: %i"), playerIndex);

    USTRChara* chara = SpawnChara(DebugP1, "P1", charaIndex);

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
    int32 keyIndex;

    if (charaIndex >= m_charaList.Num() || charaIndex < 0)
    {
        return;
    }

    InKeyMappings.Find(InKey, keyIndex);

    switch(keyIndex)
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
            uint8 key = FMath::Max(keyIndex - 4, 0);

            if (key > 4)
            {
                break;
            }

            m_charaList[charaIndex]->PlayerButton(keyIndex - 4, InPressed);

            break;
        }
    }
}

USTRChara* ASTRGameMode::SpawnChara(FString InCharaName, FString InCharaLayer, int32& OutCharaIndex)
{
    if (!CharaSets.Contains(InCharaName))
    {
        OutCharaIndex = -1;

        return nullptr;
    }

    USTRChara* chara = NewObject<USTRChara>();
    int32 charaIndex = m_charaList.Num();

    FActorSpawnParameters spawnInfo;

    ASTRCharaRenderer* renderer = GetWorld()->SpawnActor<ASTRCharaRenderer>(ASTRCharaRenderer::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, spawnInfo);

    chara->PreInit(this, charaIndex, InCharaLayer, renderer, CharaSets[InCharaName]);
    chara->Init();

    m_charaList.Add(chara);

    OutCharaIndex = charaIndex;

    return chara;
}

TArray<USTRChara*> ASTRGameMode::GetOpponentCharaList(FString InLayer, bool InContainsIgnore)
{
    TArray<USTRChara*> result;

    for(USTRChara* chara : m_charaList)
    {
        FString charaLayer = chara->GetLayer();

        if (InLayer == "P1" || InLayer == "P2")
        {
            FString opponentLayer = InLayer == "P1" ? "P2" : "P1";

            if (charaLayer == opponentLayer)
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
    int32 minX = InCollision.X - InCollision.Width;
    int32 minY = InCollision.Y - InCollision.Height;
    int32 maxX = InCollision.X + InCollision.Width;
    int32 maxY = InCollision.Y + InCollision.Height;

    DrawDebugLine(GetWorld(), FVector(minX, 0, minY) / float(1000), FVector(minX, 0, maxY) / float(1000), InColor, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(minX, 0, maxY) / float(1000), FVector(maxX, 0, maxY) / float(1000), InColor, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(maxX, 0, maxY) / float(1000), FVector(maxX, 0, minY) / float(1000), InColor, false, -1, 0, 3);
    DrawDebugLine(GetWorld(), FVector(maxX, 0, minY) / float(1000), FVector(minX, 0, minY) / float(1000), InColor, false, -1, 0, 3);
}
