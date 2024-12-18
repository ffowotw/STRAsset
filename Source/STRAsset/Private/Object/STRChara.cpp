#include "Object/STRChara.h"
#include "Structures/STRCostume.h"
#include "STRGameMode.h"
#include "STRScriptData.h"
#include "DataAssets/STRMeshArray.h"
#include "Renderer/STRObjectRenderer.h"
#include "Renderer/STRCharaRenderer.h"

// Init
void USTRChara::PreInit(ASTRGameMode* InGameMode, int32 InIndex, FString InLayer, ASTRObjectRenderer* InRenderer, FSTRDataSet InDataSet)
{
    m_gameMode = InGameMode;

    m_charaIndex = InIndex;
    m_layer = InLayer;

    if (InLayer == "P1")
    {
        m_facing = 1;
    }
    else if (InLayer == "P2")
    {
        m_facing = -1;
    }

    m_dataSet = InDataSet;

    m_meshSets.Empty();
    m_meshSets.Add("default", { InDataSet.Costumes[0].MeshArray->DefaultMeshSet });

    m_renderer = InRenderer;
    m_renderer->Init(InDataSet);

    CallFunction("PreInit");
}

void USTRChara::Init()
{
    StoreValue("202", 0);
    StoreValue("203", 0);
    StoreValue("204", 0);

    m_inputX = 0;
    m_inputY = 0;

    m_inputButtons.Init(0, m_gameMode->GetPlayerButtons().Num());
    m_lastFrameInputButtons.Init(0, m_gameMode->GetPlayerButtons().Num());
    m_buttonBuffer.Init(0, m_gameMode->GetPlayerButtons().Num());

    m_disableFlags = {};

    m_inputBuffer_directions = {};
    m_inputBuffer_frameCounts = {};

    m_freezeTime = 0;

    StoreValue("167", 0);
    StoreValue("168", 0);

    CallFunction("Init");

    JumpToState("CmnActStand");
    SetCharaState("STANDING");
}

// Tick
void USTRChara::InputTicking()
{
    for (int32 i = 0; i < m_buttonBuffer.Num(); i++)
    {
        if (m_buttonBuffer[i] > 0)
        {
            m_buttonBuffer[i]--;
        }
    }

    UpdatePlayerInputs();
    UpdateInputBuffer();
}

void USTRChara::EarlyTicking()
{
    Super::EarlyTicking();

    if (GetValue("167") > 0)
    {
        ModifyValue("SUB", "167", 1);
    }

    if (GetValue("168") > 0)
    {
        ModifyValue("SUB", "168", 1);
    }

    TickInputCheck();
    TickCommonActionCheck();

    if (m_inHitStun && m_hitByChara != nullptr && abs(GetValue("200") + GetValue("202")) > 1515000)
    {
        int32 m_overflowPosition = abs(GetValue("200") + GetValue("202")) - 1515000;

        if (GetValue("200") > 0)
        {
            m_overflowPosition *= -1;
        }

        m_hitByChara->ModifyValue("ADD", "200", m_overflowPosition);
    }
}

void USTRChara::LateTicking()
{
    StoreValue("200", FMath::Clamp(GetValue("200"), -1515000, 1515000));

    if (GetValue("202") != 0)
    {
        TArray<USTRChara*> charaList = m_gameMode->GetCharaList();

        int32 minX, maxX;
        bool minXSet, maxXSet;
        bool isMinX, isMaxX;

        minX = 0;
        maxX = 0;
        minXSet = false;
        maxXSet = false;
        isMinX = false;
        isMaxX = false;

        for(USTRChara* chara : charaList)
        {
            if (!minXSet || chara->GetValue("200") < minX)
            {
                minXSet = true;

                minX = chara->GetValue("200");
                
                if (chara == this)
                {
                    isMinX = true;
                }
                else
                {
                    isMinX = false;
                }
            }

            if (!maxXSet || chara->GetValue("200") > maxX)
            {
                maxXSet = true;

                maxX = chara->GetValue("200");
                
                if (chara == this)
                {
                    isMaxX = true;
                }
                else
                {
                    isMaxX = false;
                }
            }
        }

        if (abs(maxX - minX) > 1365000)
        {
            if (isMinX)
            {
                StoreValue("200", maxX - 1365000);
                minX = GetValue("200");
            }
            else if(isMaxX)
            {
                StoreValue("200", minX + 1365000);
                maxX = GetValue("200");
            }
        }
    }

    if (m_charaState == "JUMPING")
    {
        if (GetValue("201") <= 0)
        {
            StoreValue("157", 0);
            StoreValue("158", 0);
        
            StoreValue("201", 0);

            StoreValue("202", 0);
            StoreValue("203", 0);

            StoreValue("204", 0);

            if (CheckCurrentStateName("CmnActDashB"))
            {
                JumpToState("CmnActStand");
                SetCharaState("STANDING");
            }
            else
            {
                JumpToState("CmnActJumpLanding");

                if (m_inputY >= 0)
                {
                    SetCharaState("STANDING");
                }
                else
                {
                    SetCharaState("CROUCHING");
                }
            }
        }
    }
    else
    {
        m_groundedX = GetValue("200");
    }

    Super::LateTicking();
    
    if (GetScriptData()->Subroutines.Contains(m_currentStateName))
    {
        int32 subroutineLength = GetScriptData()->Subroutines[m_currentStateName].SubroutineValues.Num();

        if (m_stateExecutionIndex + 1 >= subroutineLength && m_stateExecutionCountdown == 0)
        {
            if (CheckCurrentStateName("CmnActJumpPre"))
            {
                JumpToState("CmnActJump");
                SetCharaState("JUMPING");

                if (GetValue("157") == 1)
                {
                    StoreValue("161", 0);
                }

                CopyValue("202", "VelocityX");
                CopyValue("203", "VelocityY");
                CopyValue("204", "Gravity");
            }
            else
            {
                ExitState();
            }

            StateExecution();
        }
    }
}

void USTRChara::TickInputCheck()
{
    int32 i;
    bool queuedButton = false;

    for (i = 0; i < m_buttonBuffer.Num(); i++)
    {
        if (m_buttonBuffer[i] > 0)
        {
            queuedButton = true;

            break;
        }
    }

    if (!queuedButton)
    {
        return;
    }

    for (i = m_moveKeys.Num() - 1; i >= 0; i--)
    {
        FSTRMove move = m_moves[m_moveKeys[i]];

        if (move.DisableFlag > 0 && m_disableFlags.Contains(move.DisableFlag))
        {
            continue;
        }

        if (move.CharaState != m_charaState)
        {
            continue;
        }

        if (m_moves.Contains(m_currentStateName))
        {
            if (m_state == "STARTUP_STATE")
            {
                if (move.Type != "NORMAL")
                {
                    if (GetValue("156") > 5)
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            else
            {
                if (m_hitList.Num() > 0)
                {
                    if (move.Type == "NORMAL" && !m_hitCancels.Contains(m_moveKeys[i]))
                    {
                        continue;
                    }

                    if (move.Type == "SPECIAL" && GetValue("166") != 1)
                    {
                        continue;
                    }

                    // UE_LOG(LogTemp, Warning, TEXT("HIT CANCEL"));
                }
                else
                {
                    if (GetValue("165") != 1 || !m_whiffCancels.Contains(m_moveKeys[i]))
                    {
                        continue;
                    }

                    // UE_LOG(LogTemp, Warning, TEXT("WHIFF CANCEL: %s"), *m_currentStateName);
                }
            }
        }

        if (move.Type == "SPECIAL" && CheckIsFollowupMove(m_moveKeys[i]))
        {
            if (!m_whiffCancels.Contains(m_moveKeys[i]) || !m_hitCancels.Contains(m_moveKeys[i]))
            {
                continue;
            }
        }
        
        bool successed = true;
        
        for (FString input : move.MoveInputs)
        {
            if (!CheckInput(input))
            {
                successed = false;

                break;
            }
        }

        if (!successed)
        {
            continue;
        }

        if (move.Type == "NORMAL" && GetValue("162") == 0)
        {
            continue;
        }

        if (move.Type == "SPECIAL" && GetValue("163") == 0)
        {
            continue;
        }

        JumpToState(m_moveKeys[i]);

        if (move.Type != "NORMAL")
        {
            for (int32 j = 0; j < m_buttonBuffer.Num(); j++)
            {
                m_buttonBuffer[j] = 0;
            }
        }
        
        break;
    }
}

void USTRChara::TickCommonActionCheck(bool InMoveEnded)
{
    if (m_inHitStun)
    {
        if (InMoveEnded)
        {
            m_inHitStun = false;
            m_hitByChara = nullptr;
        }
        else
        {
            return;
        }
    }

    if (m_moves.Contains(m_currentStateName))
    {
        float friction = float(GetValue("106")) / 100;
        float newVelocityX = float(GetValue("202")) * friction * friction * (float(GetValue("56")) / 100);

        StoreValue("202", FMath::FloorToInt(newVelocityX));

        if (abs(GetValue("202")) <= GetValue("102"))
        {
            StoreValue("202", 0);
        }

        if (!InMoveEnded)
        {
            return;
        }
    }

    if (GetValue("158") > 0)
    {
        ModifyValue("SUB", "202", GetValue("202") / 50);

        if (GetValue("203") > 0)
        {
            ModifyValue("SUB", "203", GetValue("203") / 2);
        }
        else
        {
            StoreValue("203", 0);
        }

        ModifyValue("SUB", "158", 1);

        if (GetValue("158") == GetValue("159") - 5)
        {
            StoreValue("203", 0);
        }
        else if (GetValue("158") == 0)
        {
            JumpToState("CmnActJump");
            GotoLabel("down");

            ModifyValue("SUB", "202", GetValue("202") / 4);

            if (GetValue("201") > 0)
            {
                CopyValue("204", GetValue("157") == 1 ? "117" : "113");
            }
            else
            {
                StoreValue("201", 0);
                StoreValue("203", 0);
                StoreValue("204", 0);
            }

            if (GetValue("151") <= 0)
            {
                StoreValue("161", 0);
            }
        }

        return;
    }

    if (GetValue("160") > 0)
    {
        ModifyValue("SUB", "160", 1);

        return;
    }

    if (CheckCurrentStateName("CmnActFDash"))
    {
        if (m_inputX * m_facing <= 0)
        {
            JumpToState("CmnActFDashStop");

            return;
        }

        ModifyValue("ADD", "202", GetValue("105") * m_facing);
        ModifyValue("SUB", "202", GetValue("202") / GetValue("106"));
    }
    else if (CheckCurrentStateName("CmnActFDashStop"))
    {
        float friction = float(GetValue("106")) / 100;
        float newVelocityX = float(GetValue("202")) * friction * friction;

        StoreValue("202", FMath::FloorToInt(newVelocityX));

        if (GetValue("156") >= 15)
        {
            StoreValue("202", 0);
        }
    }
    else if (CheckCurrentStateName("CmnActJumpPre"))
    {
        return;
    }
    
    FString newState = "";
    int32 subroutineLength = GetScriptData()->Subroutines[m_currentStateName].SubroutineValues.Num();

    if (m_inputY > 0)
    {
        if (GetValue("161") == 1)
        {
            if (m_charaState != "JUMPING" && (GetValue("161") == 1 || m_moves.Contains(m_currentStateName) && GetValue("164") == 1))
            {
                if (CheckInput("INPUT_CHARGE_DOWN_UP_1F"))
                {
                    StoreValue("157", 1);

                    if (m_inputX * m_facing > 0)
                    {
                        newState = "CmnFHighJump";
                    }
                    else if (m_inputX * m_facing < 0)
                    {
                        newState = "CmnBHighJump";
                    }
                    else
                    {
                        newState = "CmnVHighJump";
                    }
                }
                else
                {
                    if (m_inputX * m_facing > 0)
                    {
                        newState = "CmnFJump";
                    }
                    else if (m_inputX * m_facing < 0)
                    {
                        newState = "CmnBJump";
                    }
                    else
                    {
                        newState = "CmnVJump";
                    }
                }
            }
            else if (GetInputBufferLength() >= 2)
            {
                int8 direction, lastDirection;
                uint32 frameCount, lastFrameCount;
                int8 horizontal, vertical, lastHorizontal, lastVertical;

                GetInputFromBuffer(GetInputBufferLength() - 1, m_facing, direction, frameCount);
                GetInputFromBuffer(GetInputBufferLength() - 2, m_facing, lastDirection, lastFrameCount);

                GetAxisFromDirection(direction, horizontal, vertical);
                GetAxisFromDirection(lastDirection, lastHorizontal, lastVertical);

                if (frameCount == 1)
                {
                    if (lastVertical < 1)
                    {
                        if (horizontal > 0)
                        {
                            newState = "CmnFJump";
                        }
                        else if (horizontal < 0)
                        {
                            newState = "CmnBJump";
                        }
                        else
                        {
                            newState = "CmnVJump";
                        }
                    }
                }
            }
        }
    }
    else if (!(CheckCurrentStateName("CmnActFDashStop") && GetValue("202") > 0))
    {
        if (m_inputX * m_facing > 0)
        {
            if (m_inputY == 0)
            {
                if (CheckInput("INPUT_DASH") || CheckCurrentStateName("CmnActFDash"))
                {
                    if (m_charaState != "JUMPING")
                    {
                        newState = "CmnFDash";
                    }
                    else
                    {
                        if (GetValue("152") > 0)
                        {
                            if((GetValue("203") > 0 && GetValue("201") >= GetValue("118")) || GetValue("203") <= 0)
                            {
                                newState = "CmnFAirDash";
                            }
                        }
                    }
                }
                else if (m_charaState != "JUMPING")
                {
                    newState = "CmnFWalk";
                }
            }
            else
            {
                if (m_inputY == -1 && !CheckCurrentStateName("CmnActFDash"))
                {
                    newState = "CmnNeutral";
                }
            }
        }
        else if (m_inputX * m_facing < 0)
        {
            if (m_inputY == 0)
            {
                if (CheckInput("INPUT_BDASH"))
                {
                    if (m_charaState != "JUMPING")
                    {
                        newState = "CmnBDash";
                    }
                    else
                    {
                        if (GetValue("152") > 0 && GetValue("201") >= GetValue("118"))
                        {
                            newState = "CmnBAirDash";
                        }
                    }
                }
                else if (m_charaState != "JUMPING")
                {
                    newState = "CmnBWalk";
                }
            }
            else
            {
                newState = "CmnNeutral";
            }
        }
        else if (m_charaState != "JUMPING")
        {
            if (!CheckCurrentStateName("CmnActFDashStop") || InMoveEnded)
            {
                newState = "CmnNeutral";
            }
        }
    }

    if (newState == "" || CheckCurrentStateName(newState))
    {
        return;
    }

    if (m_moves.Contains(m_currentStateName))
    {
        if (!(m_stateExecutionIndex + 1 >= subroutineLength && m_stateExecutionCountdown == 0))
        {
            return;
        }
    }

    if (newState == "CmnNeutral")
    {
        StoreValue("202", 0);

        if (m_charaState == "STANDING")
        {
            if (CheckCurrentStateName("CmnActCrouch"))
            {
                JumpToState("CmnActCrouchToStand");
            }
            else if (!CheckCurrentStateName("CmnActCrouchToStand") && !CheckCurrentStateName("CmnActStand"))
            {
                JumpToState("CmnActStand");
            }
        }
        else if (m_charaState == "CROUCHING")
        {
            if (CheckCurrentStateName("CmnActStand"))
            {
                JumpToState("CmnActStandToCrouch");
            }
            else if (!CheckCurrentStateName("CmnActStandToCrouch") && !CheckCurrentStateName("CmnActCrouch"))
            {
                JumpToState("CmnActCrouch");
            }
        }
        else
        {
            JumpToState("CmnActStand");
        }
    }
    else if (newState == "CmnFWalk")
    {
        if (CheckCurrentStateName("CmnActFWalk"))
        {
            return;
        }

        JumpToState("CmnActFWalk");
        SetCharaState("STANDING");

        StoreValue("202", GetValue("102") * m_facing);
    }
    else if (newState == "CmnFDash")
    {
        if (CheckCurrentStateName("CmnActFDash"))
        {
            return;
        }

        JumpToState("CmnActFDash");
        SetCharaState("STANDING");

        StoreValue("202", GetValue("104") * m_facing);
    }
    else if (newState == "CmnBWalk")
    {
        if (CheckCurrentStateName("CmnActBWalk"))
        {
            return;
        }

        JumpToState("CmnActBWalk");
        SetCharaState("STANDING");

        StoreValue("202", GetValue("103") * m_facing);
    }
    else if (newState == "CmnBDash")
    {
        if (CheckCurrentStateName("CmnActBDash"))
        {
            return;
        }

        JumpToState("CmnActBDash");
        SetCharaState("JUMPING");

        StoreValue("161", 0);

        StoreValue("202", GetValue("107") * m_facing);
        CopyValue("203", "108");
        CopyValue("204", "109");
    }
    else if (newState == "CmnVJump")
    {
        if (m_charaState == "JUMPING")
        {
            JumpToState("CmnActJump");

            ModifyValue("SUB", "151", 1);
            StoreValue("152", 0);

            if (GetValue("151") <= 0)
            {
                StoreValue("161", 0);
            }

            StoreValue("202", 0);
            CopyValue("203", "112");
            CopyValue("204", "113");

            CheckFacing();
        }
        else
        {
            if (CheckCurrentStateName("CmnActJumpPre"))
            {
                return;
            }

            JumpToState("CmnActJumpPre");
            StoreValue("157", 0);

            RestoreAirJump();
            RestoreAirDash();

            StoreValue("VelocityX", 0);
            CopyValue("VelocityY", "112");
        }

        CopyValue("Gravity", "113");
    }
    else if (newState == "CmnFJump")
    {
        if (m_charaState == "JUMPING")
        {
            JumpToState("CmnActJump");

            ModifyValue("SUB", "151", 1);
            StoreValue("152", 0);

            if (GetValue("151") <= 0)
            {
                StoreValue("161", 0);
            }

            StoreValue("202", GetValue("110") * m_facing);
            CopyValue("203", "112");
            CopyValue("204", "113");

            CheckFacing();
        }
        else
        {
            if (CheckCurrentStateName("CmnActJumpPre"))
            {
                return;
            }

            JumpToState("CmnActJumpPre");
            StoreValue("157", 0);

            RestoreAirJump();
            RestoreAirDash();

            if (abs(GetValue("202")) <= GetValue("110"))
            {
                StoreValue("VelocityX", GetValue("110") * m_facing);
            }
            else
            {
                StoreValue("VelocityX", GetValue("104") * m_facing);
            }
            
            CopyValue("VelocityY", "112");
        }
        
        CopyValue("Gravity", "113");
    }
    else if (newState == "CmnBJump")
    {
        if (m_charaState == "JUMPING")
        {
            ModifyValue("SUB", "151", 1);
            StoreValue("152", 0);

            if (GetValue("151") <= 0)
            {
                StoreValue("161", 0);
            }

            StoreValue("202", GetValue("111") * m_facing);
            CopyValue("203", "112");
            CopyValue("204", "113");

            CheckFacing();
        }
        else
        {
            if (CheckCurrentStateName("CmnActJumpPre"))
            {
                return;
            }

            JumpToState("CmnActJumpPre");
            StoreValue("157", 0);

            RestoreAirJump();
            RestoreAirDash();

            StoreValue("VelocityX", GetValue("111") * m_facing);
            CopyValue("VelocityY", "112");
        }
        
        CopyValue("Gravity", "113");
    }
    else if (newState == "CmnVHighJump")
    {
        if (CheckCurrentStateName("CmnActJumpPre"))
        {
            return;
        }

        JumpToState("CmnActJumpPre");
        StoreValue("157", 1);
        
        RestoreAirDash();

        StoreValue("151", 0);
        StoreValue("161", 0);

        StoreValue("VelocityX", 0);
        CopyValue("VelocityY", "116");
        CopyValue("Gravity", "117");
    }
    else if (newState == "CmnFHighJump")
    {
        if (CheckCurrentStateName("CmnActJumpPre"))
        {
            return;
        }

        JumpToState("CmnActJumpPre");
        StoreValue("157", 1);
        
        RestoreAirDash();

        StoreValue("151", 0);
        StoreValue("161", 0);

        StoreValue("VelocityX", GetValue("114") * m_facing);
        CopyValue("VelocityY", "116");
        CopyValue("Gravity", "117");
    }
    else if (newState == "CmnBHighJump")
    {
        if (CheckCurrentStateName("CmnActJumpPre"))
        {
            return;
        }

        JumpToState("CmnActJumpPre");
        StoreValue("157", 1);
        
        RestoreAirDash();

        StoreValue("151", 0);
        StoreValue("161", 0);

        StoreValue("VelocityX", GetValue("115") * m_facing);
        CopyValue("VelocityY", "116");
        CopyValue("Gravity", "117");
    }
    else if (newState == "CmnFAirDash")
    {
        if (CheckCurrentStateName("CmnActAirFDash"))
        {
            return;
        }

        JumpToState("CmnActAirFDash");

        CopyValue("158", "119");
        CopyValue("159", "158");
        CopyValue("160", "123");

        StoreValue("161", 0);

        ModifyValue("SUB", "151", 1);
        ModifyValue("SUB", "152", 1);

        StoreValue("202", GetValue("121") * m_facing);
        StoreValue("204", 0);
    }
    else if (newState == "CmnBAirDash")
    {
        if (CheckCurrentStateName("CmnActAirBDash"))
        {
            return;
        }

        JumpToState("CmnActAirBDash");
        
        CopyValue("158", "120");
        CopyValue("159", "158");
        CopyValue("160", "124");

        StoreValue("161", 0);
        
        ModifyValue("SUB", "151", 1);
        ModifyValue("SUB", "152", 1);

        StoreValue("202", GetValue("122") * m_facing);
        StoreValue("204", 0);
    }
}

void USTRChara::TickPushboxCheck()
{
    TArray<USTRChara*> opponentCharaList = m_gameMode->GetOpponentCharaList(m_layer);

    int32 minDistance = 0;
    int32 minIndex = -1;

    for (int32 i = 0; i < opponentCharaList.Num(); i++)
    {
        if (FSTRCollision::CheckCollide(GetPushbox(), opponentCharaList[i]->GetPushbox()))
        {
            int32 distance = opponentCharaList[i]->GetValue("200") - GetValue("200");

            if (minIndex == -1 || abs(distance) < abs(minDistance))
            {
                minDistance = distance;
                minIndex = i;
            }
        }
    }

    if (minIndex == -1)
    {
        return;
    }

    USTRChara* chara = opponentCharaList[minIndex];

    FSTRCollision pushboxA = GetPushbox();
    FSTRCollision pushboxB = chara->GetPushbox();

    int32 xA = pushboxA.X;
    int32 xB = pushboxB.X;

    if (GetValue("200") > chara->GetValue("200") || (GetValue("200") == chara->GetValue("200") && abs(m_groundedX) >= abs(chara->m_groundedX)))
    {
        if (minDistance > 0 || (minDistance == 0 && m_groundedX < chara->m_groundedX))
        {
            if (xA % 2 != 0)
            {
                xA += 1;
            }

            if (xB % 2 != 0)
            {
                xB -= 1;
            }
        }
        else
        {
            if (xA % 2 != 0)
            {
                xA -= 1;
            }

            if (xB % 2 != 0)
            {
                xB += 1;
            }
        }

        if (chara->GetValue("200") > GetValue("200"))
        {
            xA += pushboxA.Width;
        }
        else
        {
            xB += pushboxB.Width;
        }

        int32 pushDistance = (xA - xB) / 2;
        int32 offsetDistance = 0;

        ModifyValue("SUB", "200", pushDistance);

        if (abs(GetValue("200")) > 1515000)
        {
            offsetDistance = 1515000 - abs(GetValue("200"));
            offsetDistance *= GetValue("200") > 0 ? 1 : -1;

            ModifyValue("ADD", "200", offsetDistance);
        }

        chara->ModifyValue("ADD", "200", pushDistance + offsetDistance);
    }
}

void USTRChara::TickRender()
{
    ASTRCharaRenderer* renderer = Cast<ASTRCharaRenderer>(m_renderer);

    if (renderer != nullptr)
    {
        if (m_meshSets.Contains(m_currentMeshSet))
        {
            renderer->UpdateMeshSet(m_meshSets[m_currentMeshSet].Meshes, m_meshesNotDisplay);
        }
        else
        {
            renderer->UpdateMeshSet({}, m_meshesNotDisplay);
        }
    }

    Super::TickRender();
}

void USTRChara::TickDrawCollisions()
{    
    Super::TickDrawCollisions();

    m_gameMode->DrawCollision(GetPushbox(), FColor::Yellow);
}

// Facing
void USTRChara::CheckFacing()
{
    if (m_layer == "P1" || m_layer == "P2")
    {
        TArray<USTRChara*> opponents = m_gameMode->GetOpponentCharaList(m_layer);

        if (opponents.Num() >= 1)
        {
            USTRChara* chara = opponents[0];

            int32 newFacing = FMath::Clamp(chara->GetValue("200") - GetValue("200"), -1, 1);

            if (newFacing != 0 && newFacing != m_facing)
            {
                m_facing = newFacing;

                if (m_charaState == "STANDING")
                {
                    JumpToState("CmnActStandTurn");
                }
                else if (m_charaState == "CROUCHING")
                {
                    JumpToState("CmnActCrouchTurn");
                }
                else
                {
                    JumpToState("CmnActAirTurn");
                }
            }
        }
    }
}

// Input
void USTRChara::PlayerUp(bool InPressed)
{
    m_playerUp = InPressed;
}

void USTRChara::PlayerDown(bool InPressed)
{
    m_playerDown = InPressed;
}

void USTRChara::PlayerRight(bool InPressed)
{
    m_playerRight = InPressed;
}

void USTRChara::PlayerLeft(bool InPressed)
{
    m_playerLeft = InPressed;
}

void USTRChara::PlayerButton(uint8 InButton, bool InPressed)
{
    if (InButton >= m_inputButtons.Num())
    {
        return;
    }

    if (InPressed)
    {
        m_queuedPressedButtons.Add(InButton);

        if (m_queuedReleasedButtons.Contains(InButton))
        {
            m_queuedReleasedButtons.Remove(InButton);
        }
    }
    else if (!m_queuedPressedButtons.Contains(InButton))
    {
        m_queuedReleasedButtons.Add(InButton);
    }
}


void USTRChara::UpdatePlayerInputs()
{
    m_inputX = 0;
    m_inputY = 0;

    if (m_playerUp)
    {
        m_inputY++;
    }
    if (m_playerDown)
    {
        m_inputY--;
    }

    if (m_playerRight)
    {
        m_inputX++;
    }
    if (m_playerLeft)
    {
        m_inputX--;
    }

    if (m_charaState != "JUMPING" && m_inputY <= 0)
    {
        if (m_inputY == 0)
        {
            SetCharaState("STANDING");
        }
        else if (!CheckCurrentStateName("CmnActFDash"))
        {
            SetCharaState("CROUCHING");
        }
    }

    m_lastFrameInputButtons = m_inputButtons;

    for (int32 i = 0; i < m_inputButtons.Num(); i++)
    {
        if (m_queuedPressedButtons.Contains(i))
        {
            if (m_inputButtons[i] == 3 || m_inputButtons[i] == 0)
            {
                m_inputButtons[i] = 1;
                m_buttonBuffer[i] = 3;
            }
        }
        else if (m_queuedReleasedButtons.Contains(i))
        {
            if (m_inputButtons[i] == 1 || m_inputButtons[i] == 2)
            {
                m_inputButtons[i] = 3;
            }
        }
    }

    m_queuedPressedButtons.Empty();
    m_queuedReleasedButtons.Empty();
}

void USTRChara::UpdateInputBuffer()
{
    int8 direction;

    GetDirectionFromAxis(m_inputX, m_inputY, direction);
    AddInputToBuffer(direction);
}

bool USTRChara::CheckMoveInput(FSTRMove InMove)
{
    TArray<bool> results = TArray<bool> { true };
    int32 i = 0;

    if (InMove.CharaState != m_charaState)
    {
        return false;
    }

    for (FString input : InMove.MoveInputs)
    {
        if (!input.StartsWith("INPUT_"))
        {
            continue;
        }

        if (input.StartsWith("INPUT_OR"))
        {
            results.Add(true);

            i++;
        }

        CheckInput(input);
    }

    if (i == 0)
    {
        return results[0];
    }

    for (bool result : results)
    {
        if (!result)
        {
            return false;
        }
    }

    return true;
}

bool USTRChara::CheckInput(FString InInput)
{
    if (!InInput.StartsWith("INPUT_"))
    {
        return false;
    }

    int32 result;
    TArray<FString> checkingKeys;

    if (InInput == "INPUT_DASH" || InInput == "INPUT_BDASH")
    {
        if (GetInputBufferLength() <= 0)
        {
            return false;
        }

        FString input = InInput.Mid(10);

        TArray<FString> inputDirections;
        TArray<uint8> inputWindows = {0, 8, 8, 1};

        if (InInput == "INPUT_DASH")
        {
            inputDirections = {"NEUTRAL", "FORWARD", "NEUTRAL", "FORWARD"};
        }
        else
        {
            inputDirections = {"NEUTRAL", "BACK", "NEUTRAL", "BACK"};
        }

        int32 inputIndex = inputDirections.Num() - 1;
        int32 lastBufferIndex = GetInputBufferLength() - 1;

        int8 direction;
        uint32 frameCount;
        int8 horizontal, vertical;

        GetInputFromBuffer(lastBufferIndex, m_facing, direction, frameCount);
        GetAxisFromDirection(direction, horizontal, vertical);

        if (MatchDirection(inputDirections[inputIndex], horizontal, vertical))
        {
            int32 inputFrameCount = 0;

            for (int32 i = lastBufferIndex; i >= 0; i--)
            {
                GetInputFromBuffer(i, m_facing, direction, frameCount);
                GetAxisFromDirection(direction, horizontal, vertical);

                if (inputIndex > 0 && vertical < 0)
                {
                    return false;
                }

                if ((inputDirections[inputIndex] == "NEUTRAL" && horizontal == 0) || MatchDirection(inputDirections[inputIndex], horizontal, vertical))
                {
                    if (inputWindows[inputIndex] > 0 && inputFrameCount + frameCount > inputWindows[inputIndex])
                    {
                        return false;
                    }

                    if (inputIndex == 0)
                    {
                        return true;
                    }

                    inputIndex--;
                    inputFrameCount = 0;
                }
                else
                {
                    inputFrameCount += frameCount;

                    if (inputWindows[inputIndex] > 0 && inputFrameCount > inputWindows[inputIndex])
                    {
                        return false;
                    }
                }
            }
        }
    }
    else if (InInput.StartsWith("INPUT_PRESS_"))
    {
        FString input = InInput.Mid(12);
        checkingKeys = m_gameMode->GetPlayerButtons();

        if (!checkingKeys.Contains(input))
        {
            return false;
        }

        checkingKeys.Find(input, result);

        return m_buttonBuffer[result] > 0;
    }
    else if (InInput.StartsWith("INPUT_HOLD_"))
    {
        FString input = InInput.Mid(11);
        checkingKeys = m_gameMode->GetPlayerButtons();

        if (!checkingKeys.Contains(input))
        {
            return false;
        }

        checkingKeys.Find(input, result);

        return m_inputButtons[result] == 2;
    }
    else if (InInput.StartsWith("INPUT_RELEASE_"))
    {
        FString input = InInput.Mid(14);
        checkingKeys = m_gameMode->GetPlayerButtons();

        if (!checkingKeys.Contains(input))
        {
            return false;
        }

        checkingKeys.Find(input, result);

        return m_inputButtons[result] == 3;
    }
    else if (InInput.StartsWith("INPUT_ANY_"))
    {
        FString input = InInput.Mid(10);

        return CheckDirection(input);
    }
    else if (InInput.StartsWith("INPUT_NO_"))
    {
        FString input = InInput.Mid(9);

        checkingKeys = TArray<FString> {
            "FORWARD",
            "BACK",
            "UP",
            "DOWN"
        };

        if (!checkingKeys.Contains(input))
        {
            return false;
        }

        checkingKeys.Find(input, result);

        switch(result)
        {
            case 0:
            {
                return m_inputX != (1 * m_facing);
            }
            case 1:
            {
                return m_inputX != (-1 * m_facing);
            }
            case 2:
            {
                return m_inputY != 1;
            }
            case 3:
            {
                return m_inputY != -1;
            }
        }
    }
    else if (InInput.StartsWith("INPUT_CHARGE_"))
    {
        if (GetInputBufferLength() <= 0)
        {
            return false;
        }

        TArray<FString> inputs;
        InInput.Mid(13).ParseIntoArray(inputs, TEXT("_"), true);

        if (inputs.Num() != 3 || !CheckDirection(inputs[1]))
        {
            return false;
        }
        
        uint32 requiredFrameCount = GetInt(inputs[2].Mid(0, inputs[2].Len() - 1));
        uint32 lastBufferIndex = GetInputBufferLength() - 1;

        int8 direction;
        uint32 frameCount;
        int8 horizontal, vertical;

        GetInputFromBuffer(lastBufferIndex, m_facing, direction, frameCount);
        GetAxisFromDirection(direction, horizontal, vertical);

        if (MatchDirection(inputs[1], horizontal, vertical))
        {
            int32 inputFrameCount = frameCount;

            for (int32 i = lastBufferIndex - 1; i >= 0; i--)
            {
                GetInputFromBuffer(i, m_facing, direction, frameCount);
                GetAxisFromDirection(direction, horizontal, vertical);

                inputFrameCount += frameCount;

                if (MatchDirection(inputs[0], horizontal, vertical))
                {
                    return frameCount >= requiredFrameCount;
                }
                else
                {
                    if (inputFrameCount > 6)
                    {
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        if (GetInputBufferLength() <= 0)
        {
            return false;
        }

        FString input = InInput.Mid(6);

        checkingKeys = TArray<FString> {
            "74",
            "112",
            "236",
            "214",
            "623",
            "421",
            "41236",
            "63214",
            "236236",
            "214214",
            "236214",
            "214236"
        };

        if (!checkingKeys.Contains(input))
        {
            return false;
        }

        checkingKeys.Find(input, result);

        TArray<uint8> inputMotions;
        TArray<bool> motionRequriments;
        TArray<uint8> inputWindows;
        bool limitOptionalInput = false;
        bool reachedOptionalLimit = false;

        // Setup inputMotions, motionRequriments, inputWindows
        switch(result)
        {
            case 0: // 66
            {
                inputMotions = TArray<uint8> { 5, 6, 5, 6 };
                motionRequriments = TArray<bool> { true, true, true, true };
                inputWindows = TArray<uint8> { 0, 8, 8, 0 };

                break;
            }
            case 1: // 22
            {
                inputMotions = TArray<uint8> { 5, 2, 5, 2 };
                motionRequriments = TArray<bool> { true, true, true, true };
                inputWindows = TArray<uint8> { 0, 10, 10, 10 };

                break;
            }
            case 2: // 236
            {
                inputMotions = TArray<uint8> { 2, 3, 6 };
                motionRequriments = TArray<bool> { true, true, true };
                inputWindows = TArray<uint8> { 0, 10, 10 };

                break;
            }
            case 3: // 214
            {
                inputMotions = TArray<uint8> { 2, 1, 4 };
                motionRequriments = TArray<bool> { true, true, true };
                inputWindows = TArray<uint8> { 0, 10, 10 };

                break;
            }
            case 4: // 623
            {
                inputMotions = TArray<uint8> { 6, 2, 3 };
                motionRequriments = TArray<bool> { true, true, true };
                inputWindows = TArray<uint8> { 0, 12, 12 };

                break;
            }
            case 5: // 421
            {
                inputMotions = TArray<uint8> { 4, 2, 1 };
                motionRequriments = TArray<bool> { true, true, true };
                inputWindows = TArray<uint8> { 0, 12, 12 };

                break;
            }
            case 6: // 41236
            {
                inputMotions = TArray<uint8> { 4, 1, 2, 3, 6 };
                motionRequriments = TArray<bool> { true, false, true, false, true };
                inputWindows = TArray<uint8> { 0, 12, 12, 12, 12 };
                limitOptionalInput = true;
                
                break;
            }
            case 7: // 63214
            {
                inputMotions = TArray<uint8> { 6, 3, 2, 1, 4 };
                motionRequriments = TArray<bool> { true, false, true, false, true };
                inputWindows = TArray<uint8> { 0, 12, 12, 12, 12 };
                limitOptionalInput = true;
                
                break;
            }
            case 8: // 236236
            {
                inputMotions = TArray<uint8> { 2, 3, 6, 2, 3, 6 };
                motionRequriments = TArray<bool> { true, false, true, true, false, true };
                inputWindows = TArray<uint8> { 0, 10, 12, 10, 10, 10 };

                break;
            }
            case 9: // 214214
            {
                inputMotions = TArray<uint8> { 2, 1, 4, 2, 1, 4 };
                motionRequriments = TArray<bool> { true, false, true, true, false, true };
                inputWindows = TArray<uint8> { 0, 10, 12, 10, 10, 10 };

                break;
            }
            case 10: // 236214
            {
                inputMotions = TArray<uint8> { 2, 3, 6, 2, 1, 4 };
                motionRequriments = TArray<bool> { true, false, true, false, true, false, true };
                inputWindows = TArray<uint8> { 0, 10, 12, 10, 10, 10 };

                break;
            }
            case 11: // 214236
            {
                inputMotions = TArray<uint8> { 2, 1, 4, 2, 3, 6 };
                motionRequriments = TArray<bool> { true, false, true, false, true, false, true };
                inputWindows = TArray<uint8> { 0, 10, 12, 10, 10, 10 };

                break;
            }
        }
        
        int32 inputIndex = inputMotions.Num() - 1;
        int32 lastBufferIndex = GetInputBufferLength() - 1;

        int8 direction;
        uint32 frameCount;
        int32 inputFrameCount = 0;

        for (int32 i = lastBufferIndex; i >= 0; i--)
        {
            GetInputFromBuffer(i, m_facing, direction, frameCount);

            // UE_LOG(LogTemp, Warning, TEXT("[%i]: Input: %i, Frames: %i"), i, direction, frameCount);
            // UE_LOG(LogTemp, Warning, TEXT("Checking: %i >> %i"), direction, inputMotions[inputIndex]);

            if (direction == inputMotions[inputIndex])
            {
                if (inputWindows[inputIndex] > 0 && inputFrameCount + frameCount > inputWindows[inputIndex])
                {
                    return false;
                }

                if (inputIndex == 0)
                {
                    return true;
                }

                inputIndex--;
                inputFrameCount = 0;
            }
            else
            {
                inputFrameCount += frameCount;

                if (motionRequriments.Contains(false))
                {
                    int8 nextDirection;
                    uint32 nextFrameCount;

                    GetInputFromBuffer(i + 1, m_facing, nextDirection, nextFrameCount);

                    if (i < lastBufferIndex && inputMotions[inputIndex] == nextDirection)
                    {
                        if (!limitOptionalInput)
                        {
                            if (inputWindows[inputIndex] > 0 && inputFrameCount + nextFrameCount > inputWindows[inputIndex])
                            {
                                return false;
                            }

                            inputIndex--;
                            inputFrameCount = 0;
                        }
                        else
                        {
                            if (!reachedOptionalLimit)
                            {
                                if (inputWindows[inputIndex] > 0 && inputFrameCount + nextFrameCount > inputWindows[inputIndex])
                                {
                                    return false;
                                }

                                reachedOptionalLimit = true;

                                inputIndex--;
                                inputFrameCount = 0;
                            }
                            else
                            {
                                return false;
                            }
                        }
                    }
                }
                
                if (inputFrameCount > inputWindows[inputIndex])
                {
                    return false;
                }
            }
        }
    }

    return false;
}

bool USTRChara::CheckDirection(FString InDirection)
{
    int32 result;
    TArray<FString> checkingKeys = TArray<FString> {
        "FORWARD",
        "BACK",
        "UP",
        "DOWN"
    };

    if (!checkingKeys.Contains(InDirection))
    {
        return false;
    }

    checkingKeys.Find(InDirection, result);

    switch(result)
    {
        case 0:
        {
            return m_inputX == (1 * m_facing);
        }
        case 1:
        {
            return m_inputX == (-1 * m_facing);
        }
        case 2:
        {
            return m_inputY == 1;
        }
        case 3:
        {
            return m_inputY == -1;
        }
    }

    return false;
}

bool USTRChara::MatchDirection(FString InDirection, int8 InHorizontal, int8 InVertical)
{
    int32 result;
    TArray<FString> checkingKeys = TArray<FString> {
        "FORWARD",
        "BACK",
        "UP",
        "DOWN"
    };

    if (!checkingKeys.Contains(InDirection))
    {
        return false;
    }

    checkingKeys.Find(InDirection, result);

    switch(result)
    {
        case 0:
        {
            return InHorizontal > 0;
        }
        case 1:
        {
            return InHorizontal < 0;
        }
        case 2:
        {
            return InVertical > 0;
        }
        case 3:
        {
            return InVertical < 0;
        }
    }

    return false;
}

// Execution
bool USTRChara::Execute(FString InExecutionHeader, TArray<FString> InValues)
{
    // Checking
    if (InExecutionHeader == "checkOpponentChara")
    {
        // TODO: checkOpponentChara

        return true;
    }
    if (InExecutionHeader == "checkInput")
    {
        StoreValue("Tmp", CheckInput(GetString(InValues[0])) ? 1 : 0);

        return true;
    }

    if (Super::Execute(InExecutionHeader, InValues))
    {
        return true;
    }

    // Mesh Set
    if (InExecutionHeader == "setDefaultMeshSet")
    {
        m_defaultMeshSet = GetString(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "resetDefaultMeshSet")
    {
        m_defaultMeshSet = "default";

        return true;
    }
    if (InExecutionHeader == "changeDefaultMeshSet")
    {
        m_currentMeshSet = m_defaultMeshSet;
        
        return true;
    }

    return false;
}

bool USTRChara::FunctionExecutions(FString InExecutionHeader, TArray<FString> InValues)
{
    if (Super::FunctionExecutions(InExecutionHeader, InValues))
    {
        return true;
    }    
    
    // Chara Details
    if (InExecutionHeader == "charaName")
    {
        m_charaName = GetString(InValues[0]);

        return true;
    }
    
    if (InExecutionHeader == "airJumpCount")
    {
        StoreValue("100", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "airDashCount")
    {
        StoreValue("101", GetInt(InValues[0]));

        return true;
    }

    // Walk
    if (InExecutionHeader == "walkFSpeed")
    {
        StoreValue("102", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "walkBSpeed")
    {
        StoreValue("103", -GetInt(InValues[0]));

        return true;
    }

    // Dash
    if (InExecutionHeader == "dashFInitSpeed")
    {
        StoreValue("104", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "dashFAcceleration")
    {
        StoreValue("105", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "dashFriction")
    {
        StoreValue("106", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "dashBXSpeed")
    {
        StoreValue("107", -GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "dashBYSpeed")
    {
        StoreValue("108", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "dashBGravity")
    {
        StoreValue("109", GetInt(InValues[0]));

        return true;
    }

    // Jump
    if (InExecutionHeader == "jumpFXSpeed")
    {
        StoreValue("110", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "jumpBXSpeed")
    {
        StoreValue("111", -GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "jumpYSpeed")
    {
        StoreValue("112", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "jumpGravity")
    {
        StoreValue("113", GetInt(InValues[0]));

        return true;
    }

    // High Jump
    if (InExecutionHeader == "highJumpFXSpeed")
    {
        StoreValue("114", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "highJumpBXSpeed")
    {
        StoreValue("115", -GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "highJumpYSpeed")
    {
        StoreValue("116", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "highJumpGravity")
    {
        StoreValue("117", GetInt(InValues[0]));

        return true;
    }

    // Air dash
    if (InExecutionHeader == "airDashMinHeight")
    {
        StoreValue("118", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "airDashFTime")
    {
        StoreValue("119", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "airDashBTime")
    {
        StoreValue("120", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "airDashFSpeed")
    {
        StoreValue("121", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "airDashBSpeed")
    {
        StoreValue("122", -GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "airDashFNoAttackTime")
    {
        StoreValue("123", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "airDashBNoAttackTime")
    {
        StoreValue("124", GetInt(InValues[0]));

        return true;
    }

    // Push box
    if (InExecutionHeader == "pushboxWidthStand")
    {
        StoreValue("125", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "pushboxHeightStand")
    {
        StoreValue("126", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "pushboxWidthCrouch")
    {
        StoreValue("127", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "pushboxHeightCrouch")
    {
        StoreValue("128", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "pushboxWidthAir")
    {
        StoreValue("129", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "pushboxHeightAir")
    {
        StoreValue("130", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "pushboxHeightLowAir")
    {
        StoreValue("131", GetInt(InValues[0]));

        return true;
    }

    // Move
    if (InExecutionHeader == "addMove")
    {
        m_editingMove = GetString(InValues[0]);

        m_moveKeys.Add(m_editingMove);
        m_moves.Add(m_editingMove, {});
    
        return true;
    }
    if (InExecutionHeader == "endMove")
    {
        m_editingMove = "";

        return true;
    }
    if (InExecutionHeader == "moveType")
    {
        m_moves[m_editingMove].Type = GetEnum(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "charaState")
    {
        m_moves[m_editingMove].CharaState = GetEnum(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "moveInput")
    {
        m_moves[m_editingMove].MoveInputs.Add(GetEnum(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "moveRequirement")
    {
        m_moves[m_editingMove].MoveRequirement = GetEnum(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "disableMoveCanceling")
    {
        m_moves[m_editingMove].DisableMoveCanceling = GetBool(InValues);

        return true;
    }
    if (InExecutionHeader == "isFollowupMove")
    {
        m_moves[m_editingMove].IsFollowupMove = GetBool(InValues);

        return true;
    }
    if (InExecutionHeader == "disableFlag")
    {
        m_moves[m_editingMove].DisableFlag = GetInt(InValues[0]);

        return true;
    }

    // Damage sprite
    if (InExecutionHeader == "setDamageSprite")
    {
        m_damageSprites.Add(GetInt(InValues[0]), GetString(InValues[1]));

        return true;
    }

    // Mesh set
    if (InExecutionHeader == "beginMeshSet")
    {
        m_editingMeshSet = GetString(InValues[0]);
        m_meshSets.Add(m_editingMeshSet, {});

        return true;
    }
    if (InExecutionHeader == "endMeshSet")
    {
        m_editingMeshSet = "";

        return true;
    }
    if (InExecutionHeader == "addToMeshSet")
    {
        FString value = GetString(InValues[0]);

        if (!m_meshSets[m_editingMeshSet].Meshes.Contains(value))
        {
            m_meshSets[m_editingMeshSet].Meshes.Add(value);
        }

        return true;
    }
    
    /*
    // Voice
    else if (InExecutionHeader == "setupVoice")
    {
        //
    }

    // Stats skill check
    else if (InExecutionHeader == "statsSkillCheck")
    {
        m_statsSkillChecks.Add(GetInt(InValues[0]), InValues[1]);
    }
    */


   return false;
}

bool USTRChara::StateExecutions(FString InExecutionHeader, TArray<FString> InValues)
{
    if (Super::StateExecutions(InExecutionHeader, InValues))
    {
        return true;
    }

    // Animation
    if (InExecutionHeader == "playAnimation")
    {
        m_animStatment = "ANIMATION";

        m_currentAnimation = GetString(InValues[0]);
    
        return true;
    }
    if (InExecutionHeader == "playCameraAnimation")
    {
        //

        return true;
    }
    if (InExecutionHeader == "swapMeshSet")
    {
        m_currentMeshSet = GetString(InValues[0]);
        
        return true;
    }
    if (InExecutionHeader == "meshDisplay")
    {
        if (GetBool(InValues[1]))
        {
            if (m_meshesNotDisplay.Contains(GetString(InValues[0])))
            {
                m_meshesNotDisplay.Remove(GetString(InValues[0]));
            }
        }
        else
        {
            m_meshesNotDisplay.Add(GetString(InValues[0]));
        }
        
        return true;
    }
    if (InExecutionHeader == "startEyeBlink")
    {
        // TODO: startEyeBlink

        return true;
    }
    if (InExecutionHeader == "stopEyeBlink")
    {
        // TODO: stopEyeBlink
        
        return true;
    }
    // TODO: mouth things

    // Invuls
    if (InExecutionHeader == "setInvul")
    {
        FString key = "";
        FString type = GetEnum(InValues[0]);

        if (type == "STRIKE")
        {
            key = "167";
        }
        else if (type == "THROW")
        {
            key = "168";
        }

        StoreValue(key, GetBool(InValues[1]) ? -1 : 0);
    
        return true;
    }
    if (InExecutionHeader == "setInvulForTime")
    {
        FString key = "";
        FString type = GetEnum(InValues[0]);

        if (type == "STRIKE")
        {
            key = "167";
        }
        else if (type == "THROW")
        {
            key = "168";
        }

        StoreValue(key, GetInt(InValues[0]));

        return true;
    }
    
    // Restores
    if (InExecutionHeader == "m_restoreAirJump")
    {
        RestoreAirJump();

        return true;
    }
    if (InExecutionHeader == "m_restoreAirDash")
    {
        RestoreAirDash();

        return true;
    }

    // Ultimate
    if (InExecutionHeader == "hideOthers")
    {
        // TODO: need to check
        for (USTRObject* obj : m_gameMode->GetObjectList())
        {
            if (obj == this && GetBool(InValues[1]))
            {
                continue;
            }

            ASTRCharaRenderer* renderer = Cast<ASTRCharaRenderer>(obj->GetRenderer());

            if (renderer != nullptr)
            {
                renderer->SetActorHiddenInGame(GetBool(InValues));
            }
        }
    }
    if (InExecutionHeader == "ultimateFreeze")
    {
        // TODO: need to check
        for (USTRObject* obj : m_gameMode->GetObjectList())
        {
            if (obj == this && GetBool(InValues[1]))
            {
                continue;
            }

            obj->SetFreezeTime(GetInt(InValues[0]));
        }
    }
    if (InExecutionHeader == "addDelayTime")
    {
        // TODO: need to check
        m_stateExecutionCountdown += GetInt(InValues[0]);
    }

    // Sfx
    if (InExecutionHeader == "stepSfx")
    {
        return true;
    }
    if (InExecutionHeader == "commonSfx")
    {
        return true;
    }
    if (InExecutionHeader == "hitCommonSfx")
    {
        return true;
    }
    if (InExecutionHeader == "guardCommonSfx")
    {
        return true;
    }

    // Voice Line
    if (InExecutionHeader == "voiceLine")
    {
        return true;
    }
    if (InExecutionHeader == "attackVoiceLine")
    {
        return true;
    }

    // Effect
    if (InExecutionHeader == "floorEffect")
    {
        return true;
    }
    if (InExecutionHeader == "landingEffect")
    {
        return true;
    }

    // Cancels
    if (InExecutionHeader == "addWhiffCancel")
    {
        m_whiffCancels.Add(GetString(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "addHitCancel")
    {
        m_hitCancels.Add(GetString(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "removeWhiffCancel")
    {
        m_whiffCancels.Remove(GetString(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "removeHitCancel")
    {
        m_hitCancels.Remove(GetString(InValues[0]));

        return true;
    }

    // Pushbox Height
    if (InExecutionHeader == "setNoCollision")
    {
        StoreValue("169", GetBool(InValues) ? 1 : 0);
    
        return true;
    }
    if (InExecutionHeader == "setPushboxHeight")
    {
        StoreValue("154", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "setPushboxHeightLow")
    {
        StoreValue("155", GetInt(InValues[0]));

        return true;
    }
    
    // Enables
    if (InExecutionHeader == "enableJump")
    {
        StoreValue("161", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "enableNormals")
    {
        StoreValue("162", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "enableSpecials")
    {
        StoreValue("163", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "enableJumpCancel")
    {
        StoreValue("164", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "enableWhiffCancel")
    {
        StoreValue("165", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "enableSpecialCancel")
    {
        StoreValue("166", GetBool(InValues) ? 1 : 0);

        return true;
    }

    // Crouch
    if (InExecutionHeader == "setCrouch")
    {
        m_charaState = GetBool(InValues) ? "CROUCHING" : "STANDING";
        
        return true;
    }


    return false;
}

// State
void USTRChara::ResetStateValues()
{
    Super::ResetStateValues();
    
    m_state = m_moves.Contains(m_currentStateName) ? "STARTUP_STATE" : "NONE";

    // Invul
    if (GetValue("167") == -1)
    {
        StoreValue("167", 0);
    }

    if (GetValue("168") == -1)
    {
        StoreValue("168", 0);
    }

    // Pushbox    
    StoreValue("169", 0);

    // Cancels
    m_whiffCancels.Empty();
    m_hitCancels.Empty();

    // Enables
    StoreValue("161", 1);
    StoreValue("162", 1);
    StoreValue("163", 1);
    StoreValue("164", 1);
    StoreValue("165", 1);
    StoreValue("166", 1);
}
