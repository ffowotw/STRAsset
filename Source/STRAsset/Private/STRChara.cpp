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

    m_pushChecked = false;

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
                    if (move.Type == "NORMAL" && !m_hitLinkOptions.Contains(m_moveKeys[i]))
                    {
                        continue;
                    }

                    // UE_LOG(LogTemp, Warning, TEXT("HIT CANCEL"));
                }
                else
                {
                    if (!m_enableWhiffCancel || !m_whiffLinkOptions.Contains(m_moveKeys[i]))
                    {
                        continue;
                    }

                    // UE_LOG(LogTemp, Warning, TEXT("WHIFF CANCEL: %s"), *m_currentStateName);
                }
            }
        }

        if (move.Type == "SPECIAL" && CheckIsFollowupMove(m_moveKeys[i]))
        {
            if (!m_whiffLinkOptions.Contains(m_moveKeys[i]) || !m_hitLinkOptions.Contains(m_moveKeys[i]))
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

    if (hitboxes.Num() <= 0)
    {
        return;
    }

    for (USTRChara* chara : opponentCharaList)
    {
        if (m_hitCharaList.Contains(chara))
        {
            continue;
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

                    // UE_LOG(LogTemp, Warning, TEXT("%i"), hitStopFrame);
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
            "2363214",
            "2141236"
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
            case 10: // 2363214
            {
                inputMotions = TArray<uint8> { 2, 3, 6, 3, 2, 1, 4 };
                motionRequriments = TArray<bool> { true, false, true, false, true, false, true };
                inputWindows = TArray<uint8> { 0, 10, 10, 10, 10, 10, 10 };

                break;
            }
            case 11: // 2141236
            {
                inputMotions = TArray<uint8> { 2, 1, 4, 1, 2, 3, 6 };
                motionRequriments = TArray<bool> { true, false, true, false, true, false, true };
                inputWindows = TArray<uint8> { 0, 10, 10, 10, 10, 10, 10 };

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
    TArray<FString> checkingKeys;
    int32 result;

    // Checking
    checkingKeys = TArray<FString> {
        "checkCurrentStateName",
        "checkLastStateName",
        "checkOpponentChara",
        "checkInput"
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_storedVal["Tmp"] = CheckCurrentStateName(InValues[0]) ? 1 : 0;

            return;
        }
        case 1:
        {
            m_storedVal["Tmp"] = CheckLastStateName(InValues[0]) ? 1 : 0;

            return;
        }
        case 2:
        {
            return;
        }
        case 3:
        {
            m_storedVal["Tmp"] = CheckInput(GetString(InValues[0])) ? 1 : 0;

            return;
        }
    }


    // If Statement
    checkingKeys = TArray<FString> {
        "if",
        "elseif",
        "else",
        "endif"
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
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
        case 1:
        {
            m_canExecute = false;

            if (GetBool(InValues) && !m_IfStatementOperated)
            {
                m_IfStatementOperated = true;
                m_canExecute = true;
            }

            return;
        }
        case 2:
        {
            m_canExecute = false;

            if (!m_IfStatementOperated)
            {
                m_canExecute = true;
            }

            return;
        }
        case 3:
        {
            m_canExecute = true;

            return;
        }
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

    if (FunctionExecutions(InExecutionHeader, InValues))
    {
        return;
    }

    StateExecutions(InExecutionHeader, InValues);
}

bool USTRChara::FunctionExecutions(FString InExecutionHeader, TArray<FString> InValues)
{
    TArray<FString> checkingKeys;
    int32 result;

    // Chara Details
    checkingKeys = TArray<FString> {
        "charaName",
        "weight",
        "defence",
        "airJumpCount",
        "airDashCount",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_charaName = GetString(InValues[0]);

            return true;
        }
        case 1:
        {
            m_weight = GetInt(InValues[0]);

            return true;
        }
        case 2:
        {
            m_defence = GetInt(InValues[0]);

            return true;
        }
        case 3:
        {
            m_maxAirJumpCount = GetInt(InValues[0]);

            return true;
        }
        case 4:
        {
            m_maxAirDashCount = GetInt(InValues[0]);

            return true;
        }
    }


    // Walk & Dash
    checkingKeys = TArray<FString> {
        "walkFSpeed",
        "walkBSpeed",
        "dashFInitSpeed",
        "dashFAcceleration",
        "dashFriction",
        "dashBXSpeed",
        "dashBYSpeed",
        "dashBGravity",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_walkFSpeed = GetInt(InValues[0]);
        
            return true;
        }
        case 1:
        {
            m_walkBSpeed = GetInt(InValues[0]);

            return true;
        }
        case 2:
        {
            m_dashFInitSpeed = GetInt(InValues[0]);

            return true;
        }
        case 3:
        {
            m_dashFAcceleration = GetInt(InValues[0]);

            return true;
        }
        case 4:
        {
            m_dashFriction = GetInt(InValues[0]);

            return true;
        }
        case 5:
        {
            m_dashBXSpeed = GetInt(InValues[0]);

            return true;
        }
        case 6:
        {
            m_dashBYSpeed = GetInt(InValues[0]);

            return true;
        }
        case 7:
        {
            m_dashBGravity = GetInt(InValues[0]);

            return true;
        }
    }


    // Jump
    checkingKeys = TArray<FString> {
        "jumpFXSpeed",
        "jumpBXSpeed",
        "jumpYSpeed",
        "jumpGravity",
        "highJumpFXSpeed",
        "highJumpBXSpeed",
        "highJumpYSpeed",
        "highJumpGravity",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_jumpFXSpeed = GetInt(InValues[0]);
        
            return true;
        }
        case 1:
        {
            m_jumpBXSpeed = GetInt(InValues[0]);

            return true;
        }
        case 2:
        {
            m_jumpYSpeed = GetInt(InValues[0]);

            return true;
        }
        case 3:
        {
            m_jumpGravity = GetInt(InValues[0]);

            return true;
        }
        case 4:
        {
            m_highJumpFXSpeed = GetInt(InValues[0]);

            return true;
        }
        case 5:
        {
            m_highJumpBXSpeed = GetInt(InValues[0]);

            return true;
        }
        case 6:
        {
            m_highJumpYSpeed = GetInt(InValues[0]);

            return true;
        }
        case 7:
        {
            m_highJumpGravity = GetInt(InValues[0]);

            return true;
        }
    }


    // Air dash
    checkingKeys = TArray<FString> {
        "airDashMinHeight",
        "airDashFTime",
        "airDashBTime",
        "airDashFSpeed",
        "airDashBSpeed",
        "airDashFNoAttackTime",
        "airDashBNoAttackTime",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_airDashMinHeight = GetInt(InValues[0]);
        
            return true;
        }
        case 1:
        {
            m_airDashFTime = GetInt(InValues[0]);

            return true;
        }
        case 2:
        {
            m_airDashBTime = GetInt(InValues[0]);

            return true;
        }
        case 3:
        {
            m_airDashFSpeed = GetInt(InValues[0]);

            return true;
        }
        case 4:
        {
            m_airDashBSpeed = GetInt(InValues[0]);

            return true;
        }
        case 5:
        {
            m_airDashFNoAttackTime = GetInt(InValues[0]);

            return true;
        }
        case 6:
        {
            m_airDashBNoAttackTime = GetInt(InValues[0]);

            return true;
        }
    }


    // Push box
    checkingKeys = TArray<FString> {
        "pushboxWidthStand",
        "pushboxHeightStand",
        "pushboxWidthCrouch",
        "pushboxHeightCrouch",
        "pushboxWidthAir",
        "pushboxHeightAir",
        "pushboxHeightLowAir",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_pushboxWidthStand = GetInt(InValues[0]);
        
            return true;
        }
        case 1:
        {
            m_pushboxHeightStand = GetInt(InValues[0]);

            return true;
        }
        case 2:
        {
            m_pushboxWidthCrouch = GetInt(InValues[0]);

            return true;
        }
        case 3:
        {
            m_pushboxHeightCrouch = GetInt(InValues[0]);

            return true;
        }
        case 4:
        {
            m_pushboxWidthAir = GetInt(InValues[0]);

            return true;
        }
        case 5:
        {
            m_pushboxHeightAir = GetInt(InValues[0]);

            return true;
        }
        case 6:
        {
            m_pushboxHeightLowAir = GetInt(InValues[0]);

            return true;
        }
    }


    // Move
    checkingKeys = TArray<FString> {
        "addMove",
        "endMove",
        "moveType",
        "charaState",
        "moveInput",
        "moveRequirement",
        "disableMoveCanceling",
        "isFollowupMove",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_editingMove = GetString(InValues[0]);

            m_moveKeys.Add(m_editingMove);
            m_moves.Add(m_editingMove, {});
        
            return true;
        }
        case 1:
        {
            m_editingMove = "";

            return true;
        }
        case 2:
        {
            m_moves[m_editingMove].Type = GetEnum(InValues[0]);

            return true;
        }
        case 3:
        {
            m_moves[m_editingMove].CharaState = GetEnum(InValues[0]);

            return true;
        }
        case 4:
        {
            m_moves[m_editingMove].MoveInputs.Add(GetEnum(InValues[0]));

            return true;
        }
        case 5:
        {
            m_moves[m_editingMove].MoveRequirement = GetEnum(InValues[0]);

            return true;
        }
        case 6:
        {
            m_moves[m_editingMove].DisableMoveCanceling = GetBool(InValues[0]);

            return true;
        }
        case 7:
        {
            m_moves[m_editingMove].IsFollowupMove = GetBool(InValues[0]);

            return true;
        }
    }


    // Mesh set
    checkingKeys = TArray<FString> {
        "beginMeshSet",
        "endMeshSet",
        "addToMeshSet",
        "resetDefaultMeshSet",
        "changeDefaultMeshSet",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_editingMeshSet = GetString(InValues[0]);
            m_meshSets.Add(m_editingMeshSet, {});

            return true;
        }
        case 1:
        {
            m_editingMeshSet = "";

            return true;
        }
        case 2:
        {
            FString value = GetString(InValues[0]);

            if (!m_meshSets[m_editingMeshSet].Meshes.Contains(value))
            {
                m_meshSets[m_editingMeshSet].Meshes.Add(value);
            }

            return true;
        }
        case 3:
        {
            // resetDefaultMeshSet

            return true;
        }
        case 4:
        {
            // changeDefaultMeshSet

            return true;
        }
    }

    /*
    // Damage sprite
    else if (InExecutionHeader == "setDamageSprite")
    {
        m_damageSprites.Add(GetInt(InValues[0]), InValues[1]);
    }
    else if (InExecutionHeader == "setDamageSpriteEx")
    {
        m_damageSpritesEx.Add(InValues[0], InValues[1]);
    }

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
    TArray<FString> checkingKeys;
    int32 result;
    
    // State Details
    checkingKeys = TArray<FString> {
        "jumpToState",
        "exitState",
        "callFunction",
        "gotoLabel",
        "gotoLabelIf",
        "hit",
        "recoveryState"
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            JumpToState(GetString(InValues[0]));

            return;
        }
        case 1:
        {
            ExitState();

            return;
        }
        case 2:
        {
            CallFunction(InValues[0]);

            return;
        }
        case 4:
        {
            GotoLabel(GetString(InValues[0]));

            return;
        }
        case 5:
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
        case 6:
        {
            m_hitCharaList.Empty();
            m_charaStatement = "ACTIVE_STATE";

            return;
        }
        case 7:
        {
            m_charaStatement = "RECOVERY_STATE";

            return;
        }
    }


    // Invul
    checkingKeys = TArray<FString> {
        "setStrikeInvul",
        "setThrowInvul",
        "strickInvulForTime",
        "throwInvulForTime"
        "setNoCollision"
    };

    switch(result)
    {
        case 0:
        {
            m_strikeInvul = GetBool(InValues) ? -1 : 0;

            return;
        }
        case 1:
        {
            m_throwInvul = GetBool(InValues) ? -1 : 0;

            return;
        }
        case 2:
        {
            m_strikeInvul = GetInt(InValues[0]);

            return;
        }
        case 3:
        {
            m_throwInvul = GetInt(InValues[0]);

            return;
        }
        case 4:
        {
            m_noCollision = GetBool(InValues);

            return;
        }
    }

    // Throw Details
    checkingKeys = TArray<FString> {
        "canGrab",
        "throwRange",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_canGrab = GetBool(InValues[0]);
        
            return;
        }
        case 1:
        {
            m_throwRange = GetInt(InValues[0]);

            return;
        }
    }


    // Move Details
    checkingKeys = TArray<FString> {
        "damage",
        "setProration",
        "attackLevel",
        "attackAngle",
        "guardType",
        "hitStop",
        "disableHitStop",
        "m_restoreAirJump",
        "m_restoreAirDash",
        "enableJumpCancel",
        "enableSpecialCancel",
        "addWhiffCancel",
        "removeWhiffCancel",
        "addHitCancel",
        "removeHitCancel",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_damage = GetInt(InValues[0]);
        
            return;
        }
        case 1:
        {
            m_proration = float(GetInt(InValues[0])) / 100;

            return;
        }
        case 2:
        {
            m_attackLevel = GetInt(InValues[0]);

            return;
        }
        case 3:
        {
            m_attackAngle = GetInt(InValues[0]);

            return;
        }
        case 4:
        {
            m_guardType = GetEnum(InValues[0]);

            return;
        }
        case 5:
        {
            m_hitStop = GetInt(InValues[0]);

            return;
        }
        case 6:
        {
            m_disableHitStop = GetBool(InValues[0]);

            return;
        }
        case 7:
        {
            m_restoreAirJump = GetBool(InValues[0]);

            return;
        }
        case 8:
        {
            m_restoreAirDash = GetBool(InValues[0]);

            return;
        }
        case 9:
        {
            m_enabledJumpCancel = GetBool(InValues[0]);

            return;
        }
        case 10:
        {
            m_enabledSpecialCancel = GetBool(InValues[0]);

            return;
        }
        case 11:
        {
            m_whiffLinkOptions.Add(GetString(InValues[0]));

            return;
        }
        case 12:
        {
            m_whiffLinkOptions.Remove(GetString(InValues[0]));

            return;
        }
        case 13:
        {
            m_hitLinkOptions.Add(GetString(InValues[0]));

            return;
        }
        case 14:
        {
            m_hitLinkOptions.Remove(GetString(InValues[0]));

            return;
        }
    }


    // Animation
    checkingKeys = TArray<FString> {
        "sprite",
        "playAnimation",
        "swapMeshSet",
        "meshDisplay",
        "startEyeBlink",
        "stopEyeBlink",
        // mouth things
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            if (InValues[0] != "keep")
            {
                m_currentSprite = GetString(InValues[0]);
            }
            
            m_stateExecutionCountdown = GetInt(InValues[1]);

            m_renderer->SetSprite(m_currentSprite);
        
            return;
        }
        case 1:
        {
            // TODO: IDK  playAnimation

            return;
        }
        case 2:
        {
            m_currentMeshSet = InValues[0];
            
            return;
        }
        case 3:
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
        case 4:
        {
            // TODO: IDK  startEyeBlink

            return;
        }
        case 5:
        {
            // TODO: IDK  stopEyeBlink
            
            return;
        }
    }


    // Sfx
    checkingKeys = TArray<FString> {
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
    checkingKeys = TArray<FString> {
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
    

    // Physics Details
    checkingKeys = TArray<FString> {
        "setGravity",
        "resetGravity",
        "addPositionX",
        "physicsXImpulse",
        "physicsYImpulse",
        "inertiaPercent",
        "velocityXPercent",
        "velocityXPercentEachFrame",
        "velocityYPercent",
        "velocityYPercentEachFrame",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_gravity = GetInt(InValues[0]);
        
            return;
        }
        case 1:
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
        case 3:
        {
            m_positionX += GetInt(InValues[0]);
        
            return;
        }
        case 4:
        {
            m_velocityX += GetInt(InValues[0]);

            return;
        }
        case 5:
        {
            m_velocityY += GetInt(InValues[0]);

            return;
        }
        case 6:
        {
            m_inertiaPercent = GetInt(InValues[0]);
        
            return;
        }
        case 7:
        {
            m_velocityXPercent = GetInt(InValues[0]);

            return;
        }
        case 8:
        {
            m_velocityXPercentEachFrame = GetInt(InValues[0]);

            return;
        }
        case 9:
        {
            m_velocityYPercent = GetInt(InValues[0]);

            return;
        }
        case 10:
        {
            m_velocityYPercentEachFrame = GetInt(InValues[0]);

            return;
        }
    }


    // Hit Effect Details
    checkingKeys = TArray<FString> {
        "groundHitEffect",
        "airHitEffect",
        "groundCounterHitEffect",
        "airCounterHitEffect",
        "resetGroundHitEffect",
        "resetAirHitEffect",
        "resetGroundCounterHitEffect",
        "resetAirCounterHitEffect"
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_groundHitEffect = GetEnum(InValues[0]);

            return;
        }
        case 1:
        {
            m_airHitEffect = GetEnum(InValues[0]);

            return;
        }
        case 2:
        {
            m_counterGroundHitEffect = GetEnum(InValues[0]);

            return;
        }
        case 3:
        {
            m_counterAirHitEffect = GetEnum(InValues[0]);

            return;
        }
        case 4:
        {
            m_groundHitEffect = "NORMAL_UPPER";

            return;
        }
        case 5:
        {
            m_airHitEffect = "NORMAL_UPPER";

            return;
        }
        case 6:
        {
            m_counterGroundHitEffect = "NORMAL_UPPER";

            return;
        }
        case 7:
        {
            m_counterAirHitEffect = "NORMAL_UPPER";

            return;
        }
    }


    // Hit Details
    checkingKeys = TArray<FString> {
        "hitGravity",
        "hitPushbackX",
        "hitPushbackY",
        "wallStickDuration",
        "rollDuration",
        "groundBounceCount",
        "groundBounceYVelocityPercent",
        "wallBounceCount",
        "wallBounceXVelocityPercent",
        "wallBounceInCornerOnly"
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_hitGravity = GetInt(InValues[0]);
        
            return;
        }
        case 1:
        {
            m_hitPushbackX = GetInt(InValues[0]);

            return;
        }
        case 2:
        {
            m_hitPushbackY = GetInt(InValues[0]);
        
            return;
        }
        case 3:
        {
            m_wallStickDuration = GetInt(InValues[0]);

            return;
        }
        case 4:
        {
            m_rollDuration = GetInt(InValues[0]);

            return;
        }
        case 5:
        {
            m_groundBounceCount = GetInt(InValues[0]);
        
            return;
        }
        case 6:
        {
            m_groundBounceYVelocityPercent = GetInt(InValues[0]);

            return;
        }
        case 7:
        {
            m_wallBounceCount = GetInt(InValues[0]);
        
            return;
        }
        case 8:
        {
            m_wallBounceXVelocityPercent = GetInt(InValues[0]);

            return;
        }
        case 9:
        {
            m_wallBounceInCornerOnly = GetBool(InValues[0]);

            return;
        }
    }


    // Counter Hit Details
    checkingKeys = TArray<FString> {
        "counterHitRollDuration",
        "counterHitGroundBounceCount",
        "counterHitGroundBounceXVelocityPercent",
        "counterHitWallBounceCount",
        "counterHitWallBounceXVelocityPercent"
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_counterHitRollDuration = GetInt(InValues[0]);
        
            return;
        }
        case 1:
        {
            m_counterHitGroundBounceCount = GetInt(InValues[0]);

            return;
        }
        case 2:
        {
            m_counterHitGroundBounceYVelocityPercent = GetInt(InValues[0]);
        
            return;
        }
        case 3:
        {
            m_counterHitWallBounceCount = GetInt(InValues[0]);

            return;
        }
        case 4:
        {
            m_counterHitWallBounceXVelocityPercent = GetInt(InValues[0]);

            return;
        }
    }


    // Hit Air Details
    checkingKeys = TArray<FString> {
        "hitAirPushbackX",
        "hitAirPushbackY",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_hitAirPushbackX = GetInt(InValues[0]);
        
            return;
        }
        case 1:
        {
            m_hitAirPushbackY = GetInt(InValues[0]);

            return;
        }
    }


    // Counter Hit Air Details
    checkingKeys = TArray<FString> {
        "counterHitAirPushbackX",
        "counterHitAirPushbackY",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_counterHitAirPushbackX = GetInt(InValues[0]);
        
            return;
        }
        case 1:
        {
            m_counterHitAirPushbackY = GetInt(InValues[0]);

            return;
        }
    }


    // Counter Hit Air Details
    checkingKeys = TArray<FString> {
        "setPushboxHeight",
        "setPushboxHeightLow",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_pushboxHeight = GetInt(InValues[0]);
        
            return;
        }
        case 1:
        {
            m_pushboxHeightLow = GetInt(InValues[0]);

            return;
        }
    }
    

    // Enables
    checkingKeys = TArray<FString> {
        "enableJump",
        "enableNormals",
        "enableSpecials",
        "enableJumpCancel",
        "enableWhiffCancels",
        "enableSpecialCancel",
    };
    checkingKeys.Find(InExecutionHeader, result);

    switch(result)
    {
        case 0:
        {
            m_enableJump = GetBool(InValues[0]);

            return;
        }
        case 1:
        {
            m_enableNormals = GetBool(InValues[0]);
            
            return;
        }
        case 2:
        {
            m_enableSpecials = GetBool(InValues[0]);
            
            return;
        }
        case 3:
        {
            m_enableJumpCancel = GetBool(InValues[0]);
            
            return;
        }
        case 4:
        {
            m_enableWhiffCancel = GetBool(InValues[0]);
            
            return;
        }
        case 5:
        {
            m_enableSpecialCancel = GetBool(InValues[0]);
            
            return;
        }
    }

    // Others
    if (InExecutionHeader == "setCrouch")
    {
        m_charaState = GetBool(InValues) ? "CROUCHING" : "STANDING";
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
    m_canGrab = false;
    m_throwRange = 0;

    // Move Details
    m_damage = 0;
    m_proration = 100;
    m_attackLevel = 0;
    m_attackAngle = 0;
    m_guardType = "ANY";

    m_hitStop = -1;
    m_shortHitStop = false;
    m_disableHitStop = false;

    m_restoreAirJump = false;
    m_restoreAirDash = false;

    m_enabledJumpCancel = false;
    m_enabledSpecialCancel = false;

    m_whiffLinkOptions.Empty();
    m_hitLinkOptions.Empty();

    // Physics Details
    m_inertiaPercent = 100;

    m_velocityXPercent = 100;
    m_velocityYPercent = 100;
    m_velocityXPercentEachFrame = 100;
    m_velocityYPercentEachFrame = 100;

    // Hit Effect Details
    m_groundHitEffect = "NORMAL_UPPER";
    m_airHitEffect = "NORMAL_UPPER";
    m_counterGroundHitEffect = "NORMAL_UPPER";
    m_counterAirHitEffect = "NORMAL_UPPER";

    // Hit Details
    m_hitGravity = 0;

    m_hitPushbackX = 0;
    m_hitPushbackY = 0;

    m_wallStickDuration = 0;
    m_rollDuration = 0;

    m_groundBounceCount = 0;
    m_groundBounceYVelocityPercent = 0;

    m_wallBounceCount = 0;
    m_wallBounceXVelocityPercent = 0;
    m_wallBounceInCornerOnly = false;

    // Counter Hit Details
    m_counterHitRollDuration = 0;

    m_counterHitGroundBounceCount = 0;
    m_counterHitGroundBounceYVelocityPercent = 0;

    m_counterHitWallBounceCount = 0;
    m_counterHitWallBounceXVelocityPercent = 0;

    // Hit Air Details
    m_hitAirPushbackX = 0;
    m_hitAirPushbackY = 0;

    // Counter Hit Air Details
    m_counterHitAirPushbackX = 0;
    m_counterHitAirPushbackY = 0;

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
