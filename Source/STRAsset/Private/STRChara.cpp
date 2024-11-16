#include "STRChara.h"
#include "STRGameMode.h"
#include "STRTRScriptData.h"
#include "Input/STRInput.h"
#include "CharaRenderer/STRCharaRenderer.h"

void USTRChara::PreInit(ASTRGameMode* InGameMode, int32 InIndex, FString InLayer, ASTRCharaRenderer* InRenderer, FSTRCharaSet InCharaSet)
{
    m_gameMode = InGameMode;

    m_charaIndex = InIndex;
    m_charaLayer = InLayer;

    if (InLayer == "P1")
    {
        m_facing = 1;
    }
    else if (InLayer == "P2")
    {
        m_facing = -1;
    }

    m_scriptData = InCharaSet.ScriptData;
    m_collisionData = InCharaSet.CollisionData;

    m_renderer = InRenderer;
    m_renderer->Init(InCharaSet);

    CallFunction("PreInit");
}

void USTRChara::Init()
{
    m_velocityX = 0;
    m_velocityY = 0;
    m_gravity = 0;

    m_inputX = 0;
    m_inputY = 0;

    m_inputButtons = { 0, 0, 0, 0, 0 };
    m_lastFrameInputButtons = { 0, 0, 0, 0, 0 };
    m_buttonBuffer = { 0, 0, 0, 0, 0 };

    m_inputBuffer = NewObject<USTRInputBuffer>();

    CallFunction("Init");

    JumpToState("CmnActStand");
    SetCharaState("STANDING");
}


#pragma region Functions - Tick

void USTRChara::TickFacing()
{
    if ((m_charaState == "JUMPING" && !m_highJumped) || m_moves.Contains(m_currentStateName))
    {
        return;
    }

    if (m_charaLayer == "P1" || m_charaLayer == "P2")
    {
        TArray<USTRChara*> opponents = m_gameMode->GetOpponentCharaList(m_charaLayer);

        if (opponents.Num() >= 1)
        {
            USTRChara* chara = opponents[0];

            int32 newFacing = FMath::Clamp(chara->m_positionX - m_positionX, -1, 1);

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

void USTRChara::InputTicking()
{
    UpdatePlayerInputs();
    UpdateInputBuffer();
}

void USTRChara::Ticking()
{
    if (m_charaStatement == "STARTUP_STATE")
    {
        m_stateExecutedFrameCount++;
    }

    if (m_strikeInvul > 0)
    {
        m_strikeInvul--;
    }

    if (m_throwInvul > 0)
    {
        m_throwInvul--;
    }

    TickInputCheck();
    TickMoving();

    m_positionX += m_velocityX;
    m_positionX = FMath::Clamp(m_positionX, -1515000, 1515000);
    m_positionY += m_velocityY;
    m_velocityY -= m_gravity;

    if (m_velocityX != 0)
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
            if (!minXSet || chara->m_positionX < minX)
            {
                minXSet = true;

                minX = chara->m_positionX;
                
                if (chara == this)
                {
                    isMinX = true;
                }
            }

            if (!maxXSet || chara->m_positionX > maxX)
            {
                maxXSet = true;

                maxX = chara->m_positionX;
                
                if (chara == this)
                {
                    isMaxX = true;
                }
            }
        }

        if (abs(maxX - minX) > 1365000)
        {
            if (isMinX)
            {
                m_positionX = maxX - 1365000;
                minX = m_positionX;
            }
            else if(isMaxX)
            {
                m_positionX = minX + 1365000;
                maxX = m_positionX;
            }
        }
    }

    if (m_charaState == "JUMPING")
    {
        if (m_positionY <= 0)
        {
            m_highJumped = false;
            m_airDashTime = 0;
        
            m_positionY = 0;

            m_velocityX = 0;
            m_velocityY = 0;
            
            m_gravity = 0;

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
        m_groundedX = m_positionX;
    }

    StateExecution();
    
    if (m_charaLayer == "P1")
    {
        if (m_scriptData->Subroutines.Contains(m_currentStateName))
        {
            UE_LOG(LogTemp, Warning, TEXT("CURRENT: %i >> MAX: %i"), m_stateExecutionIndex, m_scriptData->Subroutines[m_currentStateName].SubroutineValues.Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("%s"), *m_currentStateName);
        }
    }

    if (m_stateExecutionCountdown <= 0)
    {
        // if (m_stateExecutionIndex + 1 >= m_scriptData->Subroutines[m_currentStateName].SubroutineValues.Num())
        // {
        //     ExitState();
        // }
    }
}

void USTRChara::ContinuousTicking()
{
    for (int32 i = 0; i < m_buttonBuffer.Num(); i++)
    {
        if (m_buttonBuffer[i] > 0)
        {
            m_buttonBuffer[i]--;
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

        if (move.CharaState != m_charaState)
        {
            continue;
        }

        if (m_moves.Contains(m_currentStateName))
        {
            if (m_charaStatement == "STARTUP_STATE")
            {
                if (move.Type != "NORMAL")
                {
                    if (m_stateExecutedFrameCount > 5)
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
                if (m_hitCharaList.Num() > 0)
                {
                    if (move.Type == "NORMAL" && !m_hitCancels.Contains(m_moveKeys[i]))
                    {
                        continue;
                    }

                    // UE_LOG(LogTemp, Warning, TEXT("HIT CANCEL"));
                }
                else
                {
                    if (!m_enableWhiffCancel || !m_whiffCancels.Contains(m_moveKeys[i]))
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

        // UE_LOG(LogTemp, Warning, TEXT("%s"), *m_moveKeys[i]);

        JumpToState(m_moveKeys[i]);

        if (move.Type == "NORMAL")
        {
            m_enableJumpCancel = true;
            m_enableWhiffCancel = true;
            m_enableSpecialCancel = true;
        }
        else
        {
            for (int32 j = 0; j < m_buttonBuffer.Num(); j++)
            {
                m_buttonBuffer[j] = 0;
            }

            m_stateExecutedFrameCount = 0;
        }
        
        break;
    }
}

void USTRChara::TickMoving()
{
    if (m_moves.Contains(m_currentStateName))
    {
        float friction = float(m_dashFriction) / 100;
        float newVelocityX = float(m_velocityX) * friction;

        m_velocityX = FMath::FloorToInt(newVelocityX);

        if (abs(m_velocityX) <= 10)
        {
            m_velocityX = 0;
        }

        return;
    }

    if (m_airDashTime > 0)
    {
        m_velocityX -= m_velocityX / 50;

        if (m_velocityY > 0)
        {
            m_velocityY -= m_velocityY / 2;
        }
        else
        {
            m_velocityY = 0;
        }

        m_airDashTime--;

        if (m_airDashTime == m_storedVal["AirDashTime"] - 5)
        {
            m_velocityY = 0;
        }
        else if (m_airDashTime == 0)
        {
            JumpToState("CmnActJump");
            GotoLabel("down");

            m_velocityX -= m_velocityX / 4;

            if (m_positionY > 0)
            {
                m_gravity = m_highJumped ? m_highJumpGravity : m_jumpGravity;
            }
            else
            {
                m_positionY = 0;
                m_velocityY = 0;
                m_gravity = 0;
            }

            if (m_airJumpCount <= 0)
            {
                m_enableJump = false;
            }
        }

        return;
    }

    if (m_airDashNoAttackTime > 0)
    {
        m_airDashNoAttackTime--;

        return;
    }

    if (CheckCurrentStateName("CmnActFDash") && m_inputX * m_facing <= 0)
    {
        JumpToState("CmnActFDashStop");

        return;
    }
    else if (CheckCurrentStateName("CmnActFDashStop"))
    {
        float friction = float(m_dashFriction) / 100;
        float newVelocityX = float(m_velocityX) * friction;

        m_velocityX = FMath::FloorToInt(newVelocityX);

        if (abs(m_velocityX) <= 10)
        {
            m_velocityX = 0;
        }
    }
    else if (CheckCurrentStateName("CmnActJumpPre"))
    {
        return;
    }
    
    FString newState = "";

    if (m_inputY > 0)
    {
        if (m_enableJump)
        {
            if (m_charaState != "JUMPING" && (m_enableJump || m_moves.Contains(m_currentStateName) && m_enableJumpCancel))
            {
                if (CheckInput("INPUT_CHARGE_DOWN_UP_1F"))
                {
                    m_highJumped = true;

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
            else if (m_inputBuffer->Num() >= 2)
            {
                int8 direction, lastDirection;
                uint32 frameCount, lastFrameCount;
                int8 horizontal, vertical, lastHorizontal, lastVertical;

                m_inputBuffer->GetInput(m_inputBuffer->Num() - 1, m_facing, direction, frameCount);
                m_inputBuffer->GetInput(m_inputBuffer->Num() - 2, m_facing, lastDirection, lastFrameCount);

                USTRInputBuffer::GetAxisFromDirection(direction, horizontal, vertical);
                USTRInputBuffer::GetAxisFromDirection(lastDirection, lastHorizontal, lastVertical);

                if (frameCount == 1)
                {
                    if (lastVertical < 1)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("AIR JUMP"));

                        if (m_inputX > 0)
                        {
                            newState = "CmnFJump";
                        }
                        else if (m_inputX < 0)
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
    else if (!CheckCurrentStateName("CmnActFDashStop"))
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
                        if (m_airDashCount > 0)
                        {
                            if((m_velocityY > 0 && m_positionY >= m_airDashMinHeight) || m_velocityY <= 0)
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
                if (m_inputY == 0 || !CheckCurrentStateName("CmnActFDash"))
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
                        if (m_airDashCount > 0 && m_positionY >= m_airDashMinHeight)
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
            if (!m_moves.Contains(m_currentStateName))
            {
                newState = "CmnNeutral";
            }
        }
    }

    if (newState == "" || CheckCurrentStateName(newState))
    {
        return;
    }

    if (newState == "CmnNeutral")
    {
        m_velocityX = 0;

        if (m_charaState == "STANDING")
        {
            if (!CheckCurrentStateName("CmnActStand"))
            {
                if (!CheckCurrentStateName("CmnActCrouchToStand"))
                {
                    JumpToState("CmnActCrouchToStand");
                }
                else
                {
                    JumpToState("CmnActStand");
                }
            }
        }
        else if (m_charaState == "CROUCHING")
        {
            if (!CheckCurrentStateName("CmnActCrouch"))
            {
                if (!CheckCurrentStateName("CmnActStandToCrouch"))
                {
                    JumpToState("CmnActStandToCrouch");
                }
                else
                {
                    JumpToState("CmnActCrouch");
                }
            }
        }
    }
    else if (newState == "CmnFWalk")
    {
        m_velocityX = m_walkFSpeed * m_facing;

        JumpToState("CmnActFWalk");
        SetCharaState("STANDING");
    }
    else if (newState == "CmnFDash")
    {
        if (abs(m_velocityX) < m_dashFInitSpeed)
        {
            m_velocityX = m_dashFInitSpeed * m_facing;
        }
        else
        {
            m_velocityX += m_dashFAcceleration * m_facing;
            m_velocityX -= m_velocityX / m_dashFriction;
        }

        JumpToState("CmnActFDash");
        SetCharaState("STANDING");
    }
    else if (newState == "CmnBWalk")
    {
        m_velocityX = -m_walkBSpeed * m_facing;

        JumpToState("CmnActBWalk");
        SetCharaState("STANDING");
    }
    else if (newState == "CmnBDash")
    {
        JumpToState("CmnActBDash");
        SetCharaState("JUMPING");

        m_enableJump = false;

        m_velocityX = -m_dashBXSpeed * m_facing;
        m_velocityY = m_dashBYSpeed;
        m_gravity = m_dashBGravity;
    }
    else if (newState == "CmnVJump")
    {
        if (m_charaState == "JUMPING")
        {
            JumpToState("CmnActJump");

            m_airJumpCount--;
            m_airDashCount = 0;

            if (m_airJumpCount <= 0)
            {
                m_enableJump = false;
            }

            m_velocityX = 0;
            m_velocityY = m_jumpYSpeed;
            m_gravity = m_jumpGravity;
        }
        else
        {
            JumpToState("CmnActJumpPre");
            m_highJumped = false;

            RestoreAirJump();
            RestoreAirDash();

            m_storedVal.Add("VelocityX", 0);
            m_storedVal.Add("VelocityY", m_jumpYSpeed);
            m_storedVal.Add("Gravity", m_jumpGravity);
        }
    }
    else if (newState == "CmnFJump")
    {
        if (m_charaState == "JUMPING")
        {
            JumpToState("CmnActJump");

            m_airJumpCount--;
            m_airDashCount = 0;

            if (m_airJumpCount <= 0)
            {
                m_enableJump = false;
            }

            m_velocityX = m_jumpFXSpeed * m_facing;
            m_velocityY = m_jumpYSpeed;
            m_gravity = m_jumpGravity;
        }
        else
        {
            JumpToState("CmnActJumpPre");
            m_highJumped = false;

            RestoreAirJump();
            RestoreAirDash();

            if (m_velocityX * m_facing < m_jumpFXSpeed)
            {
                m_storedVal.Add("VelocityX", m_jumpFXSpeed * m_facing);
            }
            else
            {
                m_storedVal.Add("VelocityX", m_dashFInitSpeed * m_facing);
            }
            
            m_storedVal.Add("VelocityY", m_jumpYSpeed);
            m_storedVal.Add("Gravity", m_jumpGravity);
        }
    }
    else if (newState == "CmnBJump")
    {
        if (m_charaState == "JUMPING")
        {
            JumpToState("CmnActJump");

            m_airJumpCount--;
            m_airDashCount = 0;

            if (m_airJumpCount <= 0)
            {
                m_enableJump = false;
            }

            m_velocityX = -m_jumpBXSpeed * m_facing;
            m_velocityY = m_jumpYSpeed;
            m_gravity = m_jumpGravity;
        }
        else
        {
            JumpToState("CmnActJumpPre");
            m_highJumped = false;

            RestoreAirJump();
            RestoreAirDash();

            m_storedVal.Add("VelocityX", -m_jumpBXSpeed * m_facing);
            m_storedVal.Add("VelocityY", m_jumpYSpeed);
            m_storedVal.Add("Gravity", m_jumpGravity);
        }
    }
    else if (newState == "CmnVHighJump")
    {
        JumpToState("CmnActJumpPre");
        m_highJumped = true;
        
        RestoreAirDash();

        m_airJumpCount = 0;
        m_enableJump = false;

        m_storedVal.Add("VelocityX", 0);
        m_storedVal.Add("VelocityY", m_highJumpYSpeed);
        m_storedVal.Add("Gravity", m_highJumpGravity);
    }
    else if (newState == "CmnFHighJump")
    {
        JumpToState("CmnActJumpPre");
        m_highJumped = true;
        
        RestoreAirDash();

        m_airJumpCount = 0;
        m_enableJump = false;

        m_storedVal.Add("VelocityX", m_highJumpFXSpeed * m_facing);
        m_storedVal.Add("VelocityY", m_highJumpYSpeed);
        m_storedVal.Add("Gravity", m_highJumpGravity);
    }
    else if (newState == "CmnBHighJump")
    {
        JumpToState("CmnActJumpPre");
        m_highJumped = true;
        
        RestoreAirDash();

        m_airJumpCount = 0;
        m_enableJump = false;

        m_storedVal.Add("VelocityX", -m_highJumpBXSpeed * m_facing);
        m_storedVal.Add("VelocityY", m_highJumpYSpeed);
        m_storedVal.Add("Gravity", m_highJumpGravity);
    }
    else if (newState == "CmnFAirDash")
    {
        JumpToState("CmnActAirFDash");
        
        m_airDashTime = m_airDashFTime;
        m_airDashNoAttackTime = m_airDashFNoAttackTime;

        m_storedVal.Add("AirDashTime", m_airDashTime);

        m_enableJump = false;

        m_airJumpCount--;
        m_airDashCount--;

        m_velocityX = m_airDashFSpeed * m_facing;
        
        m_gravity = 0;
    }
    else if (newState == "CmnBAirDash")
    {
        JumpToState("CmnActAirBDash");
        
        m_airDashTime = m_airDashBTime;
        m_airDashNoAttackTime = m_airDashBNoAttackTime;

        m_storedVal.Add("AirDashTime", m_airDashTime);

        m_enableJump = false;
        
        m_airJumpCount--;
        m_airDashCount--;

        m_velocityX = -m_airDashBSpeed * m_facing;
        m_gravity = 0;
    }
}

void USTRChara::TickPushboxCheck()
{
    TArray<USTRChara*> opponentCharaList = m_gameMode->GetOpponentCharaList(m_charaLayer);

    int32 minDistance = 0;
    int32 minIndex = -1;

    for (int32 i = 0; i < opponentCharaList.Num(); i++)
    {
        if (FSTRCollision::CheckCollide(GetPushbox(), opponentCharaList[i]->GetPushbox()))
        {
            int32 distance = opponentCharaList[i]->m_positionX - m_positionX;

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

    FSTRCollision pushboxA = GetPushbox();
    FSTRCollision pushboxB = opponentCharaList[minIndex]->GetPushbox();

    int32 xA = pushboxA.X;
    int32 xB = pushboxB.X;

    if (abs(m_groundedX) >= abs(opponentCharaList[minIndex]->m_groundedX))
    {
        if (minDistance > 0 || (minDistance == 0 && m_groundedX < opponentCharaList[minIndex]->m_groundedX))
        {
            xA += pushboxA.Width;
            xB -= pushboxB.Width;

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
            xA -= pushboxA.Width;
            xB += pushboxB.Width;

            if (xA % 2 != 0)
            {
                xA -= 1;
            }

            if (xB % 2 != 0)
            {
                xB += 1;
            }
        }

        int32 m_pushDistance = (xA - xB) / 2;
        int32 offsetDistance = 0;

        m_positionX -= m_pushDistance;

        if (abs(m_positionX) > 1515000)
        {
            offsetDistance = 1515000 - abs(m_positionX);
            offsetDistance *= m_positionX > 0 ? 1 : -1;

            m_positionX += offsetDistance;
        }

        opponentCharaList[minIndex]->m_positionX += m_pushDistance + offsetDistance;
    }
}

void USTRChara::TickHitCheck()
{
    if (m_charaStatement != "ACTIVE_STATE")
    {
        return;
    }

    TArray<USTRChara*> opponentCharaList = m_gameMode->GetOpponentCharaList(m_charaLayer);

    TArray<FSTRCollision> hitboxes = GetCollisions("HITBOX");

    if (m_isThrow)
    {
        hitboxes = {
            {
                "HIT",
                m_throwRange / 2,
                1,
                m_throwRange,
                1
            }
        };
    }
    else
    {
        hitboxes = GetCollisions("HITBOX");

        if (hitboxes.Num() <= 0)
        {
            return;
        }
    }

    for (USTRChara* chara : opponentCharaList)
    {
        if (m_hitCharaList.Contains(chara))
        {
            continue;
        }

        if (m_isThrow)
        {
            if (chara->m_throwInvul != 0)
            {
                continue;
            }
        }
        else
        {
            if (chara->m_strikeInvul != 0)
            {
                continue;
            }
        }

        bool hit = false;

        for (FSTRCollision hurtbox : chara->GetCollisions("HURTBOX"))
        {
            for (FSTRCollision hitbox : hitboxes)
            {
                if (FSTRCollision::CheckCollide(hitbox, hurtbox))
                {
                    hit = true;

                    break;
                }
            }

            if (hit)
            {
                m_hitCharaList.Add(chara);

                if (!m_disableHitStop)
                {
                    int32 hitStopFrame = m_hitStop;

                    if (hitStopFrame == -1)
                    {
                        hitStopFrame = m_gameMode->GetHitStopFrame(m_attackLevel, chara->m_charaStatement == "STARTUP_STATE" || chara->m_charaStatement == "ACTIVE_STATE");
                    }

                    m_gameMode->ApplyHitStop(hitStopFrame);
                }

                if (m_executeOnHit != "")
                {
                    JumpToState(m_executeOnHit);
                }
                else
                {
                    // TODO: OnHit
                }

                break;
            }
        }
    }
}

void USTRChara::TickDamageCheck()
{
}

void USTRChara::TickRender()
{
    m_renderer->Render(m_facing, m_positionX, m_positionY);
}

void USTRChara::TickDrawCollisions()
{
    m_gameMode->DrawCollision(GetPushbox(), FColor::Yellow);

    for (FSTRCollision collision : GetCollisions(""))
    {
        m_gameMode->DrawCollision(collision, collision.Type == "HITBOX" ? FColor::Red : FColor::Blue);
    }
}

#pragma endregion Functions - Tick


#pragma region Parameters - Player

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

#pragma endregion Parameters - Player


#pragma region Functions - Inputs

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
        else
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

    USTRInputBuffer::GetDirectionFromAxis(m_inputX, m_inputY, direction);

    m_inputBuffer->AddInput(direction);
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
        if (m_inputBuffer->Num() <= 0)
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
        int32 lastBufferIndex = m_inputBuffer->Num() - 1;

        int8 direction;
        uint32 frameCount;
        int8 horizontal, vertical;

        m_inputBuffer->GetInput(lastBufferIndex, m_facing, direction, frameCount);
        USTRInputBuffer::GetAxisFromDirection(direction, horizontal, vertical);

        if (MatchDirection(inputDirections[inputIndex], horizontal, vertical))
        {
            int32 inputFrameCount = 0;

            for (int32 i = lastBufferIndex; i >= 0; i--)
            {
                m_inputBuffer->GetInput(i, m_facing, direction, frameCount);
                USTRInputBuffer::GetAxisFromDirection(direction, horizontal, vertical);

                if (vertical < 0)
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

        checkingKeys = TArray<FString> {
            "A",
            "B",
            "C",
            "D",
            "TAUNT"
        };

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

        checkingKeys = TArray<FString> {
            "A",
            "B",
            "C",
            "D",
            "TAUNT"
        };

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

        checkingKeys = TArray<FString> {
            "A",
            "B",
            "C",
            "D",
            "TAUNT"
        };

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
        if (m_inputBuffer->Num() <= 0)
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
        uint32 lastBufferIndex = m_inputBuffer->Num() - 1;

        int8 direction;
        uint32 frameCount;
        int8 horizontal, vertical;

        m_inputBuffer->GetInput(lastBufferIndex, m_facing, direction, frameCount);
        USTRInputBuffer::GetAxisFromDirection(direction, horizontal, vertical);

        if (MatchDirection(inputs[1], horizontal, vertical))
        {
            int32 inputFrameCount = frameCount;

            for (int32 i = lastBufferIndex - 1; i >= 0; i--)
            {
                m_inputBuffer->GetInput(i, m_facing, direction, frameCount);
                USTRInputBuffer::GetAxisFromDirection(direction, horizontal, vertical);

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
        if (m_inputBuffer->Num() <= 0)
        {
            return false;
        }

        FString input = InInput.Mid(6);

        checkingKeys = TArray<FString> {
            "66",
            "22",
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
        int32 lastBufferIndex = m_inputBuffer->Num() - 1;

        int8 direction;
        uint32 frameCount;
        int32 inputFrameCount = 0;

        for (int32 i = lastBufferIndex; i >= 0; i--)
        {
            m_inputBuffer->GetInput(i, m_facing, direction, frameCount);

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

                    m_inputBuffer->GetInput(i + 1, m_facing, nextDirection, nextFrameCount);

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
                
                if (inputWindows[inputIndex] > 0 && inputFrameCount > inputWindows[inputIndex])
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

#pragma endregion Functions - Inputs


#pragma region Functions - Executions
    
void USTRChara::Execute(FString InExecutionHeader, TArray<FString> InValues)
{
    // Checking
    if (InExecutionHeader == "checkCurrentStateName")
    {
        m_storedVal["Tmp"] = CheckCurrentStateName(InValues[0]) ? 1 : 0;

        return;
    }
    if (InExecutionHeader == "checkLastStateName")
    {
        m_storedVal["Tmp"] = CheckLastStateName(InValues[0]) ? 1 : 0;

        return;
    }
    if (InExecutionHeader == "checkOpponentChara")
    {
        // TODO: checkOpponentChara

        return;
    }
    if (InExecutionHeader == "checkInput")
    {
        m_storedVal["Tmp"] = CheckInput(GetString(InValues[0])) ? 1 : 0;

        return;
    }


    // If Statement
    if (InExecutionHeader == "if")
    {
        m_IfStatementOperated = false;
        m_canExecute = false;

        if (GetBool(InValues))
        {
            m_IfStatementOperated = true;
            m_canExecute = true;
        }

        return;
    }
    if (InExecutionHeader == "elseif")
    {
        m_canExecute = false;

        if (GetBool(InValues) && !m_IfStatementOperated)
        {
            m_IfStatementOperated = true;
            m_canExecute = true;
        }

        return;
    }
    if (InExecutionHeader == "else")
    {
        m_canExecute = false;

        if (!m_IfStatementOperated)
        {
            m_canExecute = true;
        }

        return;
    }
    if (InExecutionHeader == "endif")
    {
        m_canExecute = true;

        return;
    }


    // Stop execute if not executable
    if (!m_canExecute)
    {
        return;
    }


    // Value
    if (InExecutionHeader == "storeVal")
    {
        m_storedVal[InValues[0]] = GetInt(InValues[1]);

        return;
    }

    // Executions For Function
    if (FunctionExecutions(InExecutionHeader, InValues))
    {
        return;
    }

    // Executions For State
    StateExecutions(InExecutionHeader, InValues);
}

bool USTRChara::FunctionExecutions(FString InExecutionHeader, TArray<FString> InValues)
{
    // Chara Details
    if (InExecutionHeader == "charaName")
    {
        m_charaName = GetString(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "weight")
    {
        m_weight = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "defence")
    {
        m_defence = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airJumpCount")
    {
        m_airJumpCount = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airDashCount")
    {
        m_airDashCount = GetInt(InValues[0]);

        return true;
    }


    // Walk
    if (InExecutionHeader == "walkFSpeed")
    {
        m_walkFSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "walkBSpeed")
    {
        m_walkBSpeed = GetInt(InValues[0]);

        return true;
    }

    // Dash
    if (InExecutionHeader == "dashFInitSpeed")
    {
        m_dashFInitSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "dashFAcceleration")
    {
        m_dashFAcceleration = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "dashFriction")
    {
        m_dashFriction = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "dashBXSpeed")
    {
        m_dashBXSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "dashBYSpeed")
    {
        m_dashBYSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "dashBGravity")
    {
        m_dashBGravity = GetInt(InValues[0]);

        return true;
    }


    // Jump
    if (InExecutionHeader == "jumpFXSpeed")
    {
        m_jumpFXSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "jumpBXSpeed")
    {
        m_jumpBXSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "jumpYSpeed")
    {
        m_jumpYSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "jumpGravity")
    {
        m_jumpGravity = GetInt(InValues[0]);

        return true;
    }


    // High Jump
    if (InExecutionHeader == "highJumpFXSpeed")
    {
        m_highJumpFXSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "highJumpBXSpeed")
    {
        m_highJumpBXSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "highJumpYSpeed")
    {
        m_highJumpYSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "highJumpGravity")
    {
        m_highJumpGravity = GetInt(InValues[0]);

        return true;
    }


    // Air dash
    if (InExecutionHeader == "airDashMinHeight")
    {
        m_airDashMinHeight = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airDashFTime")
    {
        m_airDashFTime = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airDashBTime")
    {
        m_airDashBTime = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airDashFSpeed")
    {
        m_airDashFSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airDashBSpeed")
    {
        m_airDashBSpeed = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airDashFNoAttackTime")
    {
        m_airDashFNoAttackTime = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airDashBNoAttackTime")
    {
        m_airDashBNoAttackTime = GetInt(InValues[0]);

        return true;
    }


    // Push box
    if (InExecutionHeader == "pushboxWidthStand")
    {
        m_pushboxWidthStand = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "pushboxHeightStand")
    {
        m_pushboxHeightStand = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "pushboxWidthCrouch")
    {
        m_pushboxWidthCrouch = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "pushboxHeightCrouch")
    {
        m_pushboxHeightCrouch = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "pushboxWidthAir")
    {
        m_pushboxWidthAir = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "pushboxHeightAir")
    {
        m_pushboxHeightAir = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "pushboxHeightLowAir")
    {
        m_pushboxHeightLowAir = GetInt(InValues[0]);

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
    if (InExecutionHeader == "resetDefaultMeshSet")
    {
        // TODO: resetDefaultMeshSet

        return true;
    }
    if (InExecutionHeader == "changeDefaultMeshSet")
    {
        // TODO: changeDefaultMeshSet
        
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

void USTRChara::StateExecutions(FString InExecutionHeader, TArray<FString> InValues)
{
    // State Details
    if (InExecutionHeader == "jumpToState")
    {
        JumpToState(GetString(InValues[0]));

        return;
    }
    if (InExecutionHeader == "exitState")
    {
        ExitState();

        return;
    }
    if (InExecutionHeader == "callFunction")
    {
        CallFunction(InValues[0]);

        return;
    }
    if (InExecutionHeader == "gotoLabel")
    {
        GotoLabel(GetString(InValues[0]));

        return;
    }
    if (InExecutionHeader == "gotoLabelIf")
    {
        TArray<FString> modifiedValues;

        for (int32 i = 1; i < InValues.Num(); i++)
        {
            modifiedValues.Add(InValues[i]);
        }

        if (GetBool(modifiedValues))
        {
            GotoLabel(GetString(InValues[0]));
        }

        return;
    }


    // Animation
    if (InExecutionHeader == "sprite")
    {
        if (InValues[0] == "null")
        {
            m_currentSprite = "";
        }
        else if (InValues[0] != "keep")
        {
            m_currentSprite = GetString(InValues[0]);
        }
        
        m_stateExecutionCountdown = GetInt(InValues[1]);

        m_renderer->SetSprite(m_currentSprite);
    
        return;
    }
    if (InExecutionHeader == "playAnimation")
    {
        // TODO: playAnimation
    
        return;
    }
    if (InExecutionHeader == "swapMeshSet")
    {
        m_currentMeshSet = InValues[0];
        
        return;
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
        
        return;
    }
    if (InExecutionHeader == "startEyeBlink")
    {
        // TODO: startEyeBlink

        return;
    }
    if (InExecutionHeader == "stopEyeBlink")
    {
        // TODO: stopEyeBlink
        
        return;
    }
    // TODO: mouth things


    // Statement
    if (InExecutionHeader == "hit")
    {
        m_hitCharaList.Empty();
        m_charaStatement = "ACTIVE_STATE";
    
        return;
    }
    if (InExecutionHeader == "recoveryState")
    {
        m_charaStatement = "RECOVERY_STATE";
    
        return;
    }


    // Invuls
    if (InExecutionHeader == "setStrikeInvul")
    {
        m_strikeInvul = GetBool(InValues);
    
        return;
    }
    if (InExecutionHeader == "setThrowInvul")
    {
        m_throwInvul = GetBool(InValues);
    
        return;
    }
    if (InExecutionHeader == "strickInvulForTime")
    {
        m_strikeInvul = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "throwInvulForTime")
    {
        m_throwInvul = GetInt(InValues[0]);
    
        return;
    }
    if (InExecutionHeader == "setNoCollision")
    {
        m_noCollision = GetBool(InValues);
    
        return;
    }

    // Throw Details
    if (InExecutionHeader == "isThrow")
    {
        m_isThrow = GetBool(InValues);
    
        return;
    }
    if (InExecutionHeader == "canThrowHitStun")
    {
        m_canThrowHitStun = GetBool(InValues);
    
        return;
    }
    if (InExecutionHeader == "throwRange")
    {
        m_throwRange = GetInt(InValues[0]);
    
        return;
    }
    if (InExecutionHeader == "executeOnHit")
    {
        m_executeOnHit = GetString(InValues[0]);
    
        return;
    }
    if (InExecutionHeader == "enemyGrabSprite")
    {
        m_enemyGrabSprite = GetInt(InValues[0]);
    
        return;
    }
    if (InExecutionHeader == "setGripPosition")
    {
        // TODO: setGripPosition

        return;
    }


    // Move Details
    if (InExecutionHeader == "damage")
    {
        m_damage = GetInt(InValues[0]);
    
        return;
    }
    if (InExecutionHeader == "minDamagePercent")
    {
        m_minDamagePercent = GetInt(InValues[0]);
    
        return;
    }
    if (InExecutionHeader == "setProration")
    {
        m_proration = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "attackLevel")
    {
        m_attackLevel = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "attackAngle")
    {
        m_attackAngle = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "guardType")
    {
        m_guardType = GetEnum(InValues[0]);

        return;
    }
    if (InExecutionHeader == "hitStop")
    {
        m_hitStop = GetInt(InValues[0]);
    
        return;
    }
    if (InExecutionHeader == "disableHitStop")
    {
        m_disableHitStop = GetBool(InValues[0]);

        return;
    }


    // Physics Details
    if (InExecutionHeader == "setGravity")
    {
        m_gravity = GetInt(InValues[0]);
        
        return;
    }
    if (InExecutionHeader == "resetGravity")
    {
        if (!m_highJumped)
        {
            m_gravity = m_jumpGravity;
        }
        else
        {
            m_gravity = m_highJumpGravity;
        }

        return;
    }
    if (InExecutionHeader == "addPositionX")
    {
        m_positionX += GetInt(InValues[0]);
        
        return;
    }
    if (InExecutionHeader == "physicsXImpulse")
    {
        m_velocityX += GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "physicsYImpulse")
    {
        m_velocityY += GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "inertiaPercent")
    {
        m_inertiaPercent = GetInt(InValues[0]);
        
        return;
    }
    if (InExecutionHeader == "velocityXPercent")
    {
        m_velocityXPercent = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "velocityYPercent")
    {
        m_velocityYPercent = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "velocityXPercentEachFrame")
    {
        m_velocityXPercentEachFrame = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "velocityYPercentEachFrame")
    {
        m_velocityYPercentEachFrame = GetInt(InValues[0]);

        return;
    }


    // Restores
    if (InExecutionHeader == "m_restoreAirJump")
    {
        // TODO: restoreAirJump

        return;
    }
    if (InExecutionHeader == "m_restoreAirDash")
    {
        // TODO: restoreAirDash

        return;
    }


    // Cancels
    if (InExecutionHeader == "addWhiffCancel")
    {
        m_whiffCancels.Add(GetString(InValues[0]));

        return;
    }
    if (InExecutionHeader == "addHitCancel")
    {
        m_hitCancels.Add(GetString(InValues[0]));

        return;
    }
    if (InExecutionHeader == "removeWhiffCancel")
    {
        m_whiffCancels.Remove(GetString(InValues[0]));

        return;
    }
    if (InExecutionHeader == "removeHitCancel")
    {
        m_hitCancels.Remove(GetString(InValues[0]));

        return;
    }

/*
    // Sfx
    checkingKeys = {
        "charaSfx",
        "stepSfx",
        "commonSfx",
        "hitCommonSfx",
        "guardCommonSfx",
        "voiceLine",
        "attackVoiceLine"
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
    case 0:
    {
        // TODO: charaSfx
    
        return;
    }
    case 1:
    {
        // TODO: stepSfx

        return;
    }
    case 2:
    {
        // TODO: commonSfx
    
        return;
    }
    case 3:
    {
        // TODO: hitCommonSfx

        return;
    }
    case 4:
    {
        // TODO: guardCommonSfx

        return;
    }
    case 5:
    {
        // TODO: voiceLine
    
        return;
    }
    case 6:
    {
        // TODO: attackVoiceLine

        return;
    }
    }


    // Effect
    checkingKeys = {
        "floorEffect",
        "landingEffect",
        "setPointFxPosition",
        "createObject",
        "createParticle"
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
    case 0:
    {
        // TODO: floorEffect
    
        return;
    }
    case 1:
    {
        // TODO: landingEffect

        return;
    }
    case 2:
    {
        // TODO: setPointFxPosition

        return;
    }
    case 3:
    {
        // TODO: createObject

        return;
    }
    case 4:
    {
        // TODO: createParticle

        return;
    }
    }
    */


    // Hit Details
    if (InExecutionHeader == "hitGravity")
    {
        m_hitGravity = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "hitPushbackX")
    {
        m_hitPushbackX = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "hitPushbackY")
    {
        m_hitPushbackY = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "hitAirPushbackX")
    {
        m_hitAirPushbackX = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "hitAirPushbackY")
    {
        m_hitAirPushbackY = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "counterHitAirPushbackX")
    {
        m_counterHitAirPushbackX = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "counterHitAirPushbackY")
    {
        m_counterHitAirPushbackY = GetInt(InValues[0]);

        return;
    }


    // Hit Effect Details
    if (InExecutionHeader == "groundHitEffect")
    {
        m_groundHitEffect = GetEnum(InValues[0]);

        return;
    }
    if (InExecutionHeader == "airHitEffect")
    {
        m_airHitEffect = GetEnum(InValues[0]);

        return;
    }
    if (InExecutionHeader == "groundCounterHitEffect")
    {
        m_groundCounterHitEffect = GetEnum(InValues[0]);

        return;
    }
    if (InExecutionHeader == "airCounterHitEffect")
    {
        m_airCounterHitEffect = GetEnum(InValues[0]);

        return;
    }
    if (InExecutionHeader == "resetGroundHitEffect")
    {
        m_groundHitEffect = "NORMAL_UPPER";

        return;
    }
    if (InExecutionHeader == "resetAirHitEffect")
    {
        m_airHitEffect = "NORMAL_UPPER";

        return;
    }
    if (InExecutionHeader == "resetGroundCounterHitEffect")
    {
        m_groundCounterHitEffect = "NORMAL_UPPER";

        return;
    }
    if (InExecutionHeader == "resetAirCounterHitEffect")
    {
        m_airCounterHitEffect = "NORMAL_UPPER";

        return;
    }


    // Roll
    if (InExecutionHeader == "rollCount")
    {
        m_rollCount = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "rollDuration")
    {
        m_rollDuration = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "counterHitRollDuration")
    {
        m_counterHitRollDuration = GetInt(InValues[0]);

        return;
    }


    // Wall Stick Details
    if (InExecutionHeader == "wallStickDuration")
    {
        m_wallStickDuration = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "counterHitWallStickDuration")
    {
        m_counterHitWallStickDuration = GetInt(InValues[0]);

        return;
    }


    // Ground Bounce Details
    if (InExecutionHeader == "groundBounceCount")
    {
        m_groundBounceCount = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "counterHitGroundBounceCount")
    {
        m_counterHitGroundBounceCount = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "groundBounceYVelocityPercent")
    {
        m_groundBounceYVelocityPercent = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "counterHitGroundBounceYVelocityPercent")
    {
        m_counterHitGroundBounceYVelocityPercent = GetInt(InValues[0]);

        return;
    }


    // Wall Bounce Details
    if (InExecutionHeader == "wallBounceInCornerOnly")
    {
        m_wallBounceInCornerOnly = GetBool(InValues);

        return;
    }
    if (InExecutionHeader == "counterHitWallBounceInCornerOnly")
    {
        m_counterHitWallBounceInCornerOnly = GetBool(InValues);

        return;
    }
    if (InExecutionHeader == "wallBounceCount")
    {
        m_wallBounceCount = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "counterHitWallBounceCount")
    {
        m_counterHitWallBounceCount = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "wallBounceXVelocityPercent")
    {
        m_wallBounceXVelocityPercent = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "counterHitWallBounceXVelocityPercent")
    {
        m_counterHitWallBounceXVelocityPercent = GetInt(InValues[0]);

        return;
    }


    // Pushbox Height Details
    if (InExecutionHeader == "setPushboxHeight")
    {
        m_pushboxHeight = GetInt(InValues[0]);

        return;
    }
    if (InExecutionHeader == "setPushboxHeightLow")
    {
        m_pushboxHeightLow = GetInt(InValues[0]);

        return;
    }
    

    // Enables
    if (InExecutionHeader == "enableJump")
    {
        m_enableJump = GetBool(InValues[0]);

        return;
    }
    if (InExecutionHeader == "enableNormals")
    {
        m_enableNormals = GetBool(InValues[0]);

        return;
    }
    if (InExecutionHeader == "enableSpecials")
    {
        m_enableSpecials = GetBool(InValues[0]);

        return;
    }
    if (InExecutionHeader == "enableJumpCancel")
    {
        m_enableJumpCancel = GetBool(InValues[0]);

        return;
    }
    if (InExecutionHeader == "enableWhiffCancel")
    {
        m_enableWhiffCancel = GetBool(InValues[0]);

        return;
    }
    if (InExecutionHeader == "enableSpecialCancel")
    {
        m_enableSpecialCancel = GetBool(InValues[0]);

        return;
    }


    // Others
    if (InExecutionHeader == "setCrouch")
    {
        m_charaState = GetBool(InValues) ? "CROUCHING" : "STANDING";
        
        return;
    }
}

#pragma endregion Functions - Executions


#pragma region Functions - States

void USTRChara::JumpToState(FString InStateName)
{
    if (InStateName == "")
    {
        m_lastStateName = m_currentStateName;
        m_currentStateName = "";

        ClearButtonBuffer();

        return;
    }

    if (!m_scriptData->Subroutines.Contains(InStateName) || m_scriptData->Subroutines[InStateName].Type != STRSubroutineType::STATE)
    {
        return;
    }

    m_lastStateName = m_currentStateName;
    m_currentStateName = InStateName;

    ResetStateValues();

    int32 i = 0;

    for (const FSTRSubroutineValue value : m_scriptData->Subroutines[m_currentStateName].SubroutineValues)
    {
        if (value.Header == "label")
        {
            m_labels.Add(GetString(value.Value), i);
        }

        i++;
    }
}

void USTRChara::ResetStateValues()
{
    // If Statement
    m_canExecute = true;

    // State Details
    m_stateExecutionIndex = 0;
    m_stateExecutionCountdown = 0;
    m_stateExecutedFrameCount = 0;
    
    m_charaStatement = m_moves.Contains(m_currentStateName) ? "STARTUP_STATE" : "NONE";

    m_labels.Empty();
    m_meshesNotDisplay.Empty();

    // Invul
    if (m_strikeInvul == -1)
    {
        m_strikeInvul = 0;
    }

    if (m_throwInvul == -1)
    {
        m_throwInvul = 0;
    }
    
    m_noCollision = false;

    // Throw Details
    m_isThrow = false;
    m_canThrowHitStun = false;
    m_throwRange = 0;
    m_executeOnHit = "";

    m_enemyGrabSprite = -1;

    // Move Details
    m_damage = 0;
    m_proration = 100;
    m_attackLevel = 0;
    m_attackAngle = 0;
    m_guardType = "ANY";

    m_hitStop = -1;
    m_shortHitStop = false;
    m_disableHitStop = false;

    // Cancels
    m_whiffCancels.Empty();
    m_hitCancels.Empty();

    // Physics Details
    m_inertiaPercent = 100;

    m_velocityXPercent = 100;
    m_velocityYPercent = 100;
    m_velocityXPercentEachFrame = 100;
    m_velocityYPercentEachFrame = 100;

    // Hit Effect Details
    m_groundHitEffect = "NORMAL_UPPER";
    m_airHitEffect = "NORMAL_UPPER";
    m_groundCounterHitEffect = "NORMAL_UPPER";
    m_airCounterHitEffect = "NORMAL_UPPER";

    // Hit Details
    m_hitGravity = 0;

    m_hitPushbackX = 0;
    m_hitPushbackY = 0;
    m_hitAirPushbackX = 0;
    m_hitAirPushbackY = 0;
    m_counterHitAirPushbackX = 0;
    m_counterHitAirPushbackY = 0;

    // Roll Details
    m_rollCount = 0;
    m_rollDuration = 0;
    m_counterHitRollDuration = 0;

    // Wall Stick Details
    m_wallStickDuration = 0;
    m_counterHitWallStickDuration = 0;

    // Ground Bounce Details
    m_groundBounceCount = 0;
    m_counterHitGroundBounceCount = 0;
    m_groundBounceYVelocityPercent = 0;
    m_counterHitGroundBounceYVelocityPercent = 0;

    // Wall Bounce Details
    m_wallBounceInCornerOnly = false;
    m_counterHitWallBounceInCornerOnly = false;
    m_wallBounceCount = 0;
    m_wallBounceXVelocityPercent = 0;
    m_counterHitWallBounceCount = 0;
    m_counterHitWallBounceXVelocityPercent = 0;

    // Enables
    if (!m_moves.Contains(m_currentStateName))
    {
        m_enableJump = true;
        m_enableNormals = true;
        m_enableSpecials = true;
    }
    else
    {
        m_enableJump = false;
        m_enableNormals = false;
        m_enableSpecials = false;
    }

    m_enableJumpCancel = false;
    m_enableWhiffCancel = false;
    m_enableSpecialCancel = false;
}

void USTRChara::StateExecution()
{
    if (m_currentStateName == "" || m_scriptData == nullptr || !m_scriptData->Subroutines.Contains(m_currentStateName))
    {
        return;
    }

    if (m_stateExecutionCountdown > 0)
    {
        m_stateExecutionCountdown--;

        if (m_stateExecutionCountdown == 0)
        {
            if (m_stateExecutionIndex + 1 >= m_scriptData->Subroutines[m_currentStateName].SubroutineValues.Num())
            {
                if (CheckCurrentStateName("CmnActJumpPre"))
                {
                    JumpToState("CmnActJump");
                    SetCharaState("JUMPING");

                    if (m_highJumped)
                    {
                        m_enableJump = false;
                    }

                    m_velocityX = m_storedVal["VelocityX"];
                    m_velocityY = m_storedVal["VelocityY"];
                    m_gravity = m_storedVal["Gravity"];

                    return;
                }

                ExitState();

                return;
            }

            m_stateExecutionIndex++;
        }
        else
        {
            return;
        }
    }

    if (m_stateExecutionIndex < 0 || m_stateExecutionIndex > m_scriptData->Subroutines[m_currentStateName].SubroutineValues.Num() - 1)
    {
        ExitState();

        return;
    }

    FString executionHeader = m_scriptData->Subroutines[m_currentStateName].SubroutineValues[m_stateExecutionIndex].Header;
    TArray<FString> values;

    m_scriptData->Subroutines[m_currentStateName].SubroutineValues[m_stateExecutionIndex].Value.ParseIntoArray(values, TEXT(","), true);

    if (executionHeader == "jumpToState")
    {
        JumpToState(values[0]);

        return;
    }
    else if (executionHeader == "exitState")
    {
        ExitState();

        return;
    }

    m_stateExecutionCountdown = 0;

    // UE_LOG(LogTemp, Warning, TEXT("Execute"));

    Execute(executionHeader, values);

    if (m_stateExecutionCountdown <= 0)
    {
        if (m_stateExecutionIndex + 1 >= m_scriptData->Subroutines[m_currentStateName].SubroutineValues.Num())
        {
            ExitState();

            return;
        }

        m_stateExecutionIndex++;

        StateExecution();
    }
}

void USTRChara::GotoLabel(FString InLabelName)
{
    m_stateExecutionIndex = m_labels[InLabelName];
    m_stateExecutionCountdown = 0;
}

#pragma endregion Functions - States


#pragma region Functions - Functions

void USTRChara::CallFunction(FString InFunctionName)
{
    if (!m_scriptData->Subroutines.Contains(InFunctionName) || m_scriptData->Subroutines[InFunctionName].Type != STRSubroutineType::FUNCTION)
    {
        return;
    }

    m_canExecute = true;

    for (FSTRSubroutineValue subroutineValue : m_scriptData->Subroutines[InFunctionName].SubroutineValues)
    {
        FString executionHeader = subroutineValue.Header;
        TArray<FString> values;

        subroutineValue.Value.ParseIntoArray(values, TEXT(","), true);

        Execute(subroutineValue.Header, values);
    }
}

#pragma endregion Functions - Functions


#pragma region Functions - Getters

bool USTRChara::GetBool(FString InString)
{
    if (InString.StartsWith("Val"))
    {
        FString key = InString.Mid(3);

        return m_storedVal[key] > 0 ? true : false;
    }

    return GetInt(InString) > 0 ? true : false;
}

bool USTRChara::GetBool(TArray<FString> InArray)
{
    if (InArray[0].StartsWith("Val"))
    {
        return m_storedVal[InArray[0]] > 0 ? true : false;
    }
    else if (InArray[0].StartsWith("(COMPARE)"))
    {
        if (InArray[1] == "==")
        {
            return GetInt(InArray[2]) == GetInt(InArray[3]);
        }
        else if (InArray[1] == "!=")
        {
            return GetInt(InArray[2]) != GetInt(InArray[3]);
        }
        else if (InArray[1] == "<")
        {
            return GetInt(InArray[2]) < GetInt(InArray[3]);
        }
        else if (InArray[1] == "<=")
        {
            return GetInt(InArray[2]) <= GetInt(InArray[3]);
        }
        else if (InArray[1] == ">")
        {
            return GetInt(InArray[2]) > GetInt(InArray[3]);
        }
        else if (InArray[1] == ">=")
        {
            return GetInt(InArray[2]) >= GetInt(InArray[3]);
        }
    }
    else if (InArray[0].StartsWith("(RNG)"))
    {
        return GetInt(InArray[1]) >= FMath::RandRange(0, GetInt(InArray[2]));
    }
    else if (InArray[0].StartsWith("(NOT)"))
    {
        return !GetBool(InArray[1]);
    }

    return GetBool(InArray[0]);
}

#pragma endregion Functions - Getters
