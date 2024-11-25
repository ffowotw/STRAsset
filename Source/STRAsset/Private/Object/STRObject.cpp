#include "Object/STRObject.h"
#include "STRGameMode.h"
#include "STRScriptData.h"
#include "DataAssets/STRParticleDataAsset.h"
#include "Particle/STRParticle.h"
#include "Renderer/STRObjectRenderer.h"

// Tick

void USTRObject::EarlyTicking()
{
    m_storedVal["frame"]++;
}

void USTRObject::Ticking()
{
    m_positionX += m_velocityX;
    m_positionY += m_velocityY;
    m_velocityY -= m_gravity;
}

void USTRObject::LateTicking()
{
    StateExecution();
}

void USTRObject::TickHitCheck()
{
    if (m_state != "ACTIVE_STATE")
    {
        return;
    }

    TArray<USTRObject*> objectList = m_gameMode->GetObjectList();

    for (USTRObject* object : objectList)
    {
        if (object == this || object->GetLayer() == m_layer)
        {
            continue;
        }
        
        if (m_hitList.Contains(object))
        {
            continue;
        }

        bool hit = false;

        if (m_isThrow)
        {
            if (object->IsA(USTRChara::StaticClass()))
            {
                USTRChara* chara = Cast<USTRChara>(object);

                if (chara == nullptr)
                {
                    continue;
                }

                if (chara->IsThrowInvul())
                {
                    continue;
                }

                TArray<FSTRCollision> hitboxes = {
                    {
                        "HIT",
                        m_throwRange / 2,
                        1,
                        m_throwRange,
                        1
                    }
                };

                if (chara->m_positionX > m_positionX)
                {
                    if (chara->m_positionX - chara->GetPushboxWidth() < m_positionX + m_throwRange)
                    {
                        hit = true;
                    }
                }
                else
                {
                    if (chara->m_positionX + chara->GetPushboxWidth() > m_positionX - m_throwRange)
                    {
                        hit = true;
                    }
                }

                if (hit)
                {
                    break;
                }
            }
        }
        else
        {
            if (object->IsA(USTRChara::StaticClass()))
            {
                USTRChara* chara = Cast<USTRChara>(object);

                if (chara == nullptr)
                {
                    continue;
                }

                if (chara->IsStrikeInvul())
                {
                    continue;
                }
            }

            for (FSTRCollision hurtbox : object->GetCollisions("HURTBOX"))
            {
                for (FSTRCollision hitbox : GetCollisions("HITBOX"))
                {
                    if (FSTRCollision::CheckCollide(hitbox, hurtbox))
                    {
                        hit = true;

                        break;
                    }
                }

                if (hit)
                {
                    break;
                }
            }
        }

        if (IsA(USTREffectObject::StaticClass()))
        {
            USTREffectObject* effectObject = Cast<USTREffectObject>(this);
            
            if (effectObject != nullptr && !effectObject->CanHit())
            {
                continue;
            }
        }

        if (hit)
        {
            m_hitList.Add(object);
            m_queuedHit.Add(object);

            if (IsA(USTREffectObject::StaticClass()))
            {
                USTREffectObject* effectObject = Cast<USTREffectObject>(this);
                
                if (effectObject != nullptr)
                {
                    effectObject->DecreaseHitNum();

                    if (object->IsA(USTREffectObject::StaticClass()))
                    {
                        USTREffectObject* hitEffectObject = Cast<USTREffectObject>(object);
                        
                        if (hitEffectObject != nullptr)
                        {
                            hitEffectObject->DecreaseHitNum();
                        }
                    }
                }
            }

            if (m_executeOnHit != "")
            {
                JumpToState(m_executeOnHit);
            }
        }
    }
}

void USTRObject::TickDamageCheck()
{
    for (int32 i = m_queuedHit.Num() - 1; i >= 0; i--)
    {
        USTRObject* hitObject = m_queuedHit[i];

        int32 hitStop = m_hitStop;

        if (hitObject->IsA(USTRChara::StaticClass()))
        {
            USTRChara* chara = Cast<USTRChara>(hitObject);

            bool air = chara->GetCharaState() == "JUMPING";
            bool counter = chara->GetCounterState() == "COUNTER_STATE";

            // Hit Effect
            if (air)
            {
                if (counter)
                {
                    chara->ApplyHitEffect(m_airHitEffect);
                }
                else
                {
                    chara->ApplyHitEffect(m_airCounterHitEffect);
                }
            }
            else
            {
                if (counter)
                {
                    chara->ApplyHitEffect(m_groundHitEffect);
                }
                else
                {
                    chara->ApplyHitEffect(m_groundCounterHitEffect);
                }
            }

            chara->JumpToHitState(m_attackLevel, Cast<USTRChara>(this));

            air = chara->GetCharaState() == "JUMPING";

            // Pushback
            if (air)
            {
                if (counter)
                {
                    chara->m_velocityX = m_counterHitAirPushbackX * m_facing;
                    chara->m_velocityY = m_counterHitAirPushbackY;
                }
                else
                {
                    chara->m_velocityX = m_hitAirPushbackX * m_facing;
                    chara->m_velocityY = m_hitAirPushbackY;
                }
            }
            else
            {
                chara->m_velocityX = m_hitPushbackX * m_facing;
                chara->m_velocityY = m_hitPushbackY;
            }

            hitStop += GetAdditionalHitStop(m_attackLevel, counter);
        }

        hitObject->m_life -= m_damage;

        if (!m_disableHitStop)
        {
            TArray<USTRObject*> objectList = m_gameMode->GetObjectList();

            SetFreezeTime(hitStop);
            hitObject->SetFreezeTime(hitStop);
        }

        OnDamageOrGuard();

        m_queuedHit.RemoveAt(i);
    }
}

void USTRObject::TickRender()
{
    if (m_renderer)
    {
        m_renderer->Render(m_facing, m_positionX, m_positionY);
    }

    for (ASTRParticle* particle : m_particles)
    {
        particle->Render(m_facing, m_positionX, m_positionY);
    }
}

void USTRObject::TickDrawCollisions()
{
    for (FSTRCollision collision : GetCollisions(""))
    {
        m_gameMode->DrawCollision(collision, collision.Type == "HITBOX" ? FColor::Red : FColor::Blue);
    }
}


// Event
void USTRObject::OnDestroy()
{
    if (!m_renderer || !m_renderer->IsValidLowLevel())
    {
        return;
    }

    m_renderer->Destroy();
}


// Execution
bool USTRObject::Execute(FString InExecutionHeader, TArray<FString> InValues)
{
    // Checking
    if (InExecutionHeader == "checkCurrentStateName")
    {
        m_storedVal["Tmp"] = CheckCurrentStateName(InValues[0]) ? 1 : 0;

        return true;
    }
    if (InExecutionHeader == "checkLastStateName")
    {
        m_storedVal["Tmp"] = CheckLastStateName(InValues[0]) ? 1 : 0;

        return true;
    }

    // If Statement
    if (InExecutionHeader == "if")
    {
        m_ifStatementLayer++;
        m_passedIfStatement.Add(false);

        m_canExecute = false;

        if (GetBool(InValues))
        {
            m_passedIfStatement[m_ifStatementLayer] = true;
            m_canExecute = true;
        }

        return true;
    }
    if (InExecutionHeader == "elif")
    {
        m_canExecute = false;

        if (GetBool(InValues) && !m_passedIfStatement[m_ifStatementLayer])
        {
            m_passedIfStatement[m_ifStatementLayer] = true;
            m_canExecute = true;
        }

        return true;
    }
    if (InExecutionHeader == "else")
    {
        m_canExecute = false;

        if (!m_passedIfStatement[m_ifStatementLayer])
        {
            m_passedIfStatement[m_ifStatementLayer] = true;
            m_canExecute = true;
        }

        return true;
    }
    if (InExecutionHeader == "endif")
    {
        m_passedIfStatement.RemoveAt(m_ifStatementLayer);
        m_ifStatementLayer--;

        m_canExecute = true;

        return true;
    }

    // Stop execute if not executable
    if (!m_canExecute)
    {
        return true;
    }


    // Value
    if (InExecutionHeader == "storeVal")
    {
        m_storedVal[InValues[0]] = GetInt(InValues[1]);

        return true;
    }


    // Executions For Function
    if (FunctionExecutions(InExecutionHeader, InValues))
    {
        return true;
    }

    // Executions For State
    if (StateExecutions(InExecutionHeader, InValues))
    {
        return true;
    }
    

    return false;
}

bool USTRObject::FunctionExecutions(FString InExecutionHeader, TArray<FString> InValues)
{
    if (InExecutionHeader == "life")
    {
        m_life = GetInt(InValues[0]);

        return true;
    }

    return false;
}

bool USTRObject::StateExecutions(FString InExecutionHeader, TArray<FString> InValues)
{
    // Function Relative
    if (InExecutionHeader == "callFunction")
    {
        CallFunction(InValues[0]);

        return true;
    }

    // State
    if (InExecutionHeader == "jumpToState")
    {
        JumpToState(GetString(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "exitState")
    {
        ExitState();

        return true;
    }
    
    // Label
    if (InExecutionHeader == "gotoLabel")
    {
        GotoLabel(GetString(InValues[0]));

        return true;
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

        return true;
    }

    // Animation
    if (InExecutionHeader == "sprite")
    {
        m_stepFrame = true;

        if (InValues[0] == "null")
        {
            m_currentSprite = "";
        }
        else if (InValues[0] != "keep")
        {
            m_currentSprite = GetString(InValues[0]);
        }
        
        m_stateExecutionCountdown = GetInt(InValues[1]);

        if (m_renderer)
        {
            m_renderer->SetSprite(m_currentSprite);
        }
    
        return true;
    }

    // Statement
    if (InExecutionHeader == "hit")
    {
        m_hitList.Empty();
        m_queuedHit.Empty();
        
        m_state = "ACTIVE_STATE";
    
        return true;
    }
    if (InExecutionHeader == "recoveryState")
    {
        m_state = "RECOVERY_STATE";
    
        return true;
    }

    // Sfx TODO
    if (InExecutionHeader == "playSfx")
    {
        return true;
    }
    if (InExecutionHeader == "charaSfx")
    {
        return true;
    }

    // "setPointFxPosition",

    // Object TODO
    if (InExecutionHeader == "createObject")
    {
        m_gameMode->CreateObject(this, GetString(InValues[0]), m_dataSet);

        return true;
    }
    if (InExecutionHeader == "linkObjectCollision")
    {
        return true;
    }

    // Particle
    if (InExecutionHeader == "createParticle")
    {
        m_gameMode->CreateParticle(m_dataSet.ParticleDataAsset, GetString(InValues[0]));

        return true;
    }

    // Physics
    if (InExecutionHeader == "addPositionX")
    {
        m_positionX += GetInt(InValues[0]);
        
        return true;
    }
    if (InExecutionHeader == "physicsXImpulse")
    {
        m_velocityX += GetInt(InValues[0]) * m_facing;

        return true;
    }
    if (InExecutionHeader == "physicsYImpulse")
    {
        m_velocityY += GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "velocityXPercent")
    {
        m_velocityXPercent = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "velocityYPercent")
    {
        m_velocityYPercent = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "velocityXPercentEachFrame")
    {
        m_velocityXPercentEachFrame = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "velocityYPercentEachFrame")
    {
        m_velocityYPercentEachFrame = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "setGravity")
    {
        m_gravity = GetInt(InValues[0]);
        
        return true;
    }
    if (InExecutionHeader == "resetGravity")
    {
        if (m_storedVal.Contains("Gravity"))
        {
            m_gravity = m_storedVal["Gravity"];
        }
        else
        {
            m_gravity = 0;
        }

        return true;
    }
    
    // Throw
    if (InExecutionHeader == "isThrow")
    {
        m_isThrow = GetBool(InValues);
    
        return true;
    }
    if (InExecutionHeader == "canThrowHitStun")
    {
        m_canThrowHitStun = GetBool(InValues);
    
        return true;
    }
    if (InExecutionHeader == "throwRange")
    {
        m_throwRange = GetInt(InValues[0]);
    
        return true;
    }
    if (InExecutionHeader == "executeOnHit")
    {
        m_executeOnHit = GetString(InValues[0]);
    
        return true;
    }
    if (InExecutionHeader == "enemyGrabSprite")
    {
        m_enemyGrabSprite = GetInt(InValues[0]);
    
        return true;
    }
    if (InExecutionHeader == "setGripPosition")
    {
        // TODO: setGripPosition

        return true;
    }

    // Damage
    if (InExecutionHeader == "damage")
    {
        m_damage = GetInt(InValues[0]);
    
        return true;
    }
    if (InExecutionHeader == "setProration")
    {
        m_proration = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "minDamagePercent")
    {
        m_minDamagePercent = GetInt(InValues[0]);
    
        return true;
    }
    
    // Attack
    if (InExecutionHeader == "attackLevel")
    {
        SetAttackLevel(GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "attackAngle")
    {
        m_attackAngle = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "isAirUnblockable")
    {
        m_isAirUnblockable = GetBool(InValues);

        return true;
    }

    // Attack Disable
    if (InExecutionHeader == "noPushbackScaling")
    {
        m_noPushbackScaling = GetBool(InValues);

        return true;
    }
    if (InExecutionHeader == "noHitstunScaling")
    {
        m_noHitstunScaling = GetBool(InValues);

        return true;
    }
    if (InExecutionHeader == "noGravityScaling")
    {
        m_noGravityScaling = GetBool(InValues);

        return true;
    }
    
    // Types
    if (InExecutionHeader == "guardType")
    {
        m_guardType = GetEnum(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitType")
    {
        m_counterHitType = GetEnum(InValues[0]);

        return true;
    }

    // Hit
    if (InExecutionHeader == "hitGravity")
    {
        m_hitGravity = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "hitStop")
    {
        if (GetInt(InValues[0]) >= 0)
        {
            m_hitStop = GetInt(InValues[0]);
        }
    
        return true;
    }
    if (InExecutionHeader == "disableHitStop")
    {
        m_disableHitStop = GetBool(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "hitPushbackX")
    {
        m_hitPushbackX = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "hitPushbackY")
    {
        m_hitPushbackY = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "hitAirPushbackX")
    {
        m_hitAirPushbackX = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "hitAirPushbackY")
    {
        m_hitAirPushbackY = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitAirPushbackX")
    {
        m_counterHitAirPushbackX = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitAirPushbackY")
    {
        m_counterHitAirPushbackY = GetInt(InValues[0]);

        return true;
    }

    // Hit Effect
    if (InExecutionHeader == "groundHitEffect")
    {
        m_groundHitEffect = GetEnum(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airHitEffect")
    {
        m_airHitEffect = GetEnum(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "groundCounterHitEffect")
    {
        m_groundCounterHitEffect = GetEnum(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "airCounterHitEffect")
    {
        m_airCounterHitEffect = GetEnum(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "resetGroundHitEffect")
    {
        m_groundHitEffect = "NORMAL_UPPER";

        return true;
    }
    if (InExecutionHeader == "resetAirHitEffect")
    {
        m_airHitEffect = "NORMAL_UPPER";

        return true;
    }
    if (InExecutionHeader == "resetGroundCounterHitEffect")
    {
        m_groundCounterHitEffect = "NORMAL_UPPER";

        return true;
    }
    if (InExecutionHeader == "resetAirCounterHitEffect")
    {
        m_airCounterHitEffect = "NORMAL_UPPER";

        return true;
    }

    // Roll
    if (InExecutionHeader == "rollCount")
    {
        m_rollCount = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "rollDuration")
    {
        m_rollDuration = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitRollDuration")
    {
        m_counterHitRollDuration = GetInt(InValues[0]);

        return true;
    }

    // Wall Stick
    if (InExecutionHeader == "wallStickDuration")
    {
        m_wallStickDuration = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitWallStickDuration")
    {
        m_counterHitWallStickDuration = GetInt(InValues[0]);

        return true;
    }

    // Ground Bounce
    if (InExecutionHeader == "groundBounceCount")
    {
        m_groundBounceCount = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitGroundBounceCount")
    {
        m_counterHitGroundBounceCount = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "groundBounceYVelocityPercent")
    {
        m_groundBounceYVelocityPercent = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitGroundBounceYVelocityPercent")
    {
        m_counterHitGroundBounceYVelocityPercent = GetInt(InValues[0]);

        return true;
    }

    // Wall Bounce
    if (InExecutionHeader == "wallBounceInCornerOnly")
    {
        m_wallBounceInCornerOnly = GetBool(InValues);

        return true;
    }
    if (InExecutionHeader == "counterHitWallBounceInCornerOnly")
    {
        m_counterHitWallBounceInCornerOnly = GetBool(InValues);

        return true;
    }
    if (InExecutionHeader == "wallBounceCount")
    {
        m_wallBounceCount = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitWallBounceCount")
    {
        m_counterHitWallBounceCount = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "wallBounceXVelocityPercent")
    {
        m_wallBounceXVelocityPercent = GetInt(InValues[0]);

        return true;
    }
    if (InExecutionHeader == "counterHitWallBounceXVelocityPercent")
    {
        m_counterHitWallBounceXVelocityPercent = GetInt(InValues[0]);

        return true;
    }

    // Ignore
    if (InExecutionHeader == "ignoreSpeed")
    {
        //
    }

    return false;
}

// State
void USTRObject::JumpToState(FString InStateName)
{
    if (InStateName == "")
    {
        m_lastStateName = m_currentStateName;
        m_currentStateName = "";

        return;
    }

    if (!GetScriptData()->Subroutines.Contains(InStateName) || GetScriptData()->Subroutines[InStateName].Type != STRSubroutineType::STATE)
    {
        return;
    }

    m_lastStateName = m_currentStateName;
    m_currentStateName = InStateName;

    ResetStateValues();

    int32 i = 0;

    for (const FSTRSubroutineValue value : GetScriptData()->Subroutines[m_currentStateName].SubroutineValues)
    {
        if (value.Header == "label")
        {
            m_labels.Add(GetString(value.Value), i);
        }

        i++;
    }
}

void USTRObject::ResetStateValues()
{
    // If Statement
    m_ifStatementLayer = -1;
    m_passedIfStatement.Empty();
    m_canExecute = true;

    // State Details
    m_stateExecutionIndex = 0;
    m_stateExecutionCountdown = 0;

    if (!m_storedVal.Contains("frame"))
    {
        m_storedVal.Add("frame", 0);
    }
    else
    {
        m_storedVal["frame"] = 0;
    }
    
    m_state = "STARTUP_STATE";

    m_labels.Empty();

    // Move Details
    m_damage = 0;
    m_proration = 65;
    m_attackLevel = 0;
    m_attackAngle = 0;
    m_guardType = "ANY";

    m_hitStop = -1;
    m_shortHitStop = false;
    m_disableHitStop = false;

    // Physics Details
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
}

void USTRObject::StateExecution()
{
    if (m_currentStateName == "" || GetScriptData() == nullptr || !GetScriptData()->Subroutines.Contains(m_currentStateName))
    {
        return;
    }

    int32 subroutineLength = GetScriptData()->Subroutines[m_currentStateName].SubroutineValues.Num();

    if (m_stateExecutionCountdown > 0)
    {
        m_stateExecutionCountdown--;

        if (m_stateExecutionCountdown == 0)
        {
            if (m_stateExecutionIndex + 1 >= subroutineLength)
            {
                return;
            }

            m_stateExecutionIndex++;
        }
        else
        {
            return;
        }
    }

    if (m_stateExecutionIndex < 0 || m_stateExecutionIndex >= subroutineLength)
    {
        ExitState();

        return;
    }

    FString executionHeader = GetScriptData()->Subroutines[m_currentStateName].SubroutineValues[m_stateExecutionIndex].Header;
    TArray<FString> values;

    GetScriptData()->Subroutines[m_currentStateName].SubroutineValues[m_stateExecutionIndex].Value.ParseIntoArray(values, TEXT(","), true);

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
        if (m_stateExecutionIndex >= subroutineLength)
        {
            return;
        }

        m_stateExecutionIndex++;

        StateExecution();
    }
}

void USTRObject::GotoLabel(FString InLabelName)
{
    m_stateExecutionIndex = m_labels[InLabelName];
    m_stateExecutionCountdown = 0;
}

// Function
void USTRObject::CallFunction(FString InFunctionName)
{
    if (!GetScriptData()->Subroutines.Contains(InFunctionName) || GetScriptData()->Subroutines[InFunctionName].Type != STRSubroutineType::FUNCTION)
    {
        return;
    }

    m_canExecute = true;

    for (FSTRSubroutineValue subroutineValue : GetScriptData()->Subroutines[InFunctionName].SubroutineValues)
    {
        FString executionHeader = subroutineValue.Header;
        TArray<FString> values;

        subroutineValue.Value.ParseIntoArray(values, TEXT(","), true);

        Execute(subroutineValue.Header, values);
    }
}
