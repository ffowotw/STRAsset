#include "Object/STRObject.h"
#include "STRGameMode.h"
#include "STRScriptData.h"
#include "DataAssets/STRParticleDataAsset.h"
#include "Particle/STRParticle.h"
#include "Renderer/STRObjectRenderer.h"

// Tick

void USTRObject::EarlyTicking()
{
    ModifyValue("Add", "156", 1);
}

void USTRObject::Ticking()
{
    StoreValue("202", GetValue("202") * GetValue("53") / 100);
    StoreValue("203", GetValue("203") * GetValue("54") / 100);

    ModifyValue("ADD", "200", GetValue("202") * GetValue("50") / 100);
    ModifyValue("ADD", "201", GetValue("203") * GetValue("51") / 100);
    ModifyValue("SUB", "203", GetValue("204"));
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

        if (GetValue("57") == 1)
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

                if (chara->GetValue("200") > GetValue("200"))
                {
                    if (chara->GetValue("200") - chara->GetPushboxWidth() < GetValue("200") + GetValue("60"))
                    {
                        hit = true;
                    }
                }
                else
                {
                    if (chara->GetValue("200") + chara->GetPushboxWidth() > GetValue("200") - GetValue("60"))
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

        int32 hitStop = GetValue("68");

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

            chara->JumpToHitState(GetValue("64"), Cast<USTRChara>(this));

            air = chara->GetCharaState() == "JUMPING";

            // Pushback
            if (air)
            {
                if (counter)
                {
                    chara->StoreValue("202", GetValue("74") * m_facing);
                    chara->StoreValue("203", GetValue("75"));
                }
                else
                {
                    chara->StoreValue("202", GetValue("72") * m_facing);
                    chara->StoreValue("203", GetValue("73"));
                }
            }
            else
            {
                chara->StoreValue("202", GetValue("70") * m_facing);
                chara->StoreValue("203", GetValue("71"));
            }

            hitStop += GetAdditionalHitStop(GetValue("64"), counter);
        }

        hitObject->ModifyValue("SUB", "150", GetValue("61"));

        if (GetValue("69") != 1)
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
        m_renderer->Render(m_facing, GetValue("200"), GetValue("201"));
    }

    for (ASTRParticle* particle : m_particles)
    {
        particle->Render(m_facing, GetValue("200"), GetValue("201"));
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
        StoreValue("Tmp", CheckCurrentStateName(InValues[0]) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "checkLastStateName")
    {
        StoreValue("Tmp", CheckLastStateName(InValues[0]) ? 1 : 0);

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
        StoreValue(GetString(InValues[0]), GetInt(InValues[1]));

        return true;
    }
    if (InExecutionHeader == "modifyVal")
    {
        ModifyValue(GetEnum(InValues[0]), GetString(InValues[1]), GetInt(InValues[2]));
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
        StoreValue("0", GetInt(InValues[0]));
        StoreValue("150", GetInt(InValues[0]));

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
        m_animStatment = "SPRITE";

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
        ModifyValue("ADD", "200", GetInt(InValues[0]));
        
        return true;
    }
    if (InExecutionHeader == "physicsXImpulse")
    {
        ModifyValue("ADD", "202", GetInt(InValues[0]) * m_facing);

        return true;
    }
    if (InExecutionHeader == "physicsYImpulse")
    {
        ModifyValue("ADD", "203", GetInt(InValues[0]) * m_facing);

        return true;
    }
    if (InExecutionHeader == "setGravity")
    {
        StoreValue("204", GetInt(InValues[0]));
        
        return true;
    }
    if (InExecutionHeader == "resetGravity")
    {
        if (ContainsValue("Gravity"))
        {
            CopyValue("204", "Gravity");
        }
        else
        {
            StoreValue("204", 0);
        }

        return true;
    }

    if (InExecutionHeader == "velocityXPercent")
    {
        StoreValue("50", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "velocityYPercent")
    {
        StoreValue("51", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "gravityPercent")
    {
        StoreValue("52", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "velocityXPercentEachFrame")
    {
        StoreValue("53", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "velocityYPercentEachFrame")
    {
        StoreValue("54", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "gravityPercentEachFrame")
    {
        StoreValue("55", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "inertiaPercent")
    {
        StoreValue("56", GetInt(InValues[0]));
        
        return true;
    }
    
    // Throw
    if (InExecutionHeader == "isThrow")
    {
        StoreValue("57", GetBool(InValues) ? 1 : 0);
    
        return true;
    }
    if (InExecutionHeader == "canThrowHitStun")
    {
        StoreValue("58", GetBool(InValues) ? 1 : 0);
    
        return true;
    }
    if (InExecutionHeader == "throwRange")
    {
        StoreValue("59", GetInt(InValues[0]));
    
        return true;
    }
    if (InExecutionHeader == "executeOnHit")
    {
        // TODO: Need to Finish
        m_executeOnHit = GetString(InValues[0]);
    
        return true;
    }
    if (InExecutionHeader == "enemyGrabSprite")
    {
        StoreValue("60", GetInt(InValues[0]));
    
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
        StoreValue("61", GetInt(InValues[0]));
    
        return true;
    }
    if (InExecutionHeader == "setProration")
    {
        StoreValue("62", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "minDamagePercent")
    {
        StoreValue("63", GetInt(InValues[0]));
    
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
        StoreValue("65", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "isAirUnblockable")
    {
        StoreValue("66", GetBool(InValues) ? 1 : 0);

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
        StoreValue("67", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "hitStop")
    {
        if (GetInt(InValues[0]) >= 0)
        {
            StoreValue("68", GetInt(InValues[0]));
        }
    
        return true;
    }
    if (InExecutionHeader == "disableHitStop")
    {
        StoreValue("69", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "hitPushbackX")
    {
        StoreValue("70", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "hitPushbackY")
    {
        StoreValue("71", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "hitAirPushbackX")
    {
        StoreValue("72", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "hitAirPushbackY")
    {
        StoreValue("73", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "counterHitAirPushbackX")
    {
        StoreValue("74", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "counterHitAirPushbackY")
    {
        StoreValue("75", GetInt(InValues[0]));

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
        StoreValue("76", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "rollDuration")
    {
        StoreValue("55", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "counterHitRollDuration")
    {
        StoreValue("56", GetInt(InValues[0]));

        return true;
    }

    // Wall Stick
    if (InExecutionHeader == "wallStickDuration")
    {
        StoreValue("79", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "counterHitWallStickDuration")
    {
        StoreValue("80", GetInt(InValues[0]));

        return true;
    }

    // Ground Bounce
    if (InExecutionHeader == "groundBounceCount")
    {
        StoreValue("81", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "counterHitGroundBounceCount")
    {
        StoreValue("82", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "groundBounceYVelocityPercent")
    {
        StoreValue("83", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "counterHitGroundBounceYVelocityPercent")
    {
        StoreValue("84", GetInt(InValues[0]));

        return true;
    }

    // Wall Bounce
    if (InExecutionHeader == "wallBounceInCornerOnly")
    {
        StoreValue("85", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "counterHitWallBounceInCornerOnly")
    {
        StoreValue("86", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "wallBounceCount")
    {
        StoreValue("87", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "counterHitWallBounceCount")
    {
        StoreValue("88", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "wallBounceXVelocityPercent")
    {
        StoreValue("89", GetInt(InValues[0]));

        return true;
    }
    if (InExecutionHeader == "counterHitWallBounceXVelocityPercent")
    {
        StoreValue("90", GetInt(InValues[0]));

        return true;
    }

    if (InExecutionHeader == "noPushbackScaling")
    {
        return true;
    }
    if (InExecutionHeader == "noHitstunScaling")
    {
        return true;
    }
    if (InExecutionHeader == "noGravityScaling")
    {
        return true;
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

    StoreValue("156", 0);
    
    m_state = "STARTUP_STATE";

    m_labels.Empty();

    // Physics
    StoreValue("50", 100);
    StoreValue("51", 100);
    StoreValue("52", 100);
    StoreValue("53", 100);
    StoreValue("54", 100);
    StoreValue("55", 100);
    StoreValue("56", 100);

    // Throw Details
    StoreValue("57", 0);
    StoreValue("58", 0);
    StoreValue("59", 0);
    StoreValue("60", 0);

    m_executeOnHit = "";

    // Move Details
    StoreValue("61", 0);
    StoreValue("62", 65);
    StoreValue("63", 0);
    StoreValue("64", 0);
    StoreValue("65", 0);
    StoreValue("66", 0);

    m_guardType = "ANY";
    m_counterHitType = "DEFAULT";

    // Physics Details

    // Hit Effect Details
    m_groundHitEffect = "NORMAL_UPPER";
    m_airHitEffect = "NORMAL_UPPER";
    m_groundCounterHitEffect = "NORMAL_UPPER";
    m_airCounterHitEffect = "NORMAL_UPPER";

    // Hit Details
    StoreValue("67", 0);

    StoreValue("68", -1);
    StoreValue("69", 0);

    StoreValue("70", 0);
    StoreValue("71", 0);
    StoreValue("72", 0);
    StoreValue("73", 0);
    StoreValue("74", 0);
    StoreValue("75", 0);

    // Roll Details
    StoreValue("76", 0);
    StoreValue("55", 0);
    StoreValue("56", 0);

    // Wall Stick Details
    StoreValue("79", 0);
    StoreValue("80", 0);

    // Ground Bounce Details
    StoreValue("81", 0);
    StoreValue("82", 0);
    StoreValue("83", 0);
    StoreValue("84", 0);

    // Wall Bounce Details
    StoreValue("85", 0);
    StoreValue("86", 0);
    StoreValue("87", 0);
    StoreValue("88", 0);
    StoreValue("89", 0);
    StoreValue("90", 0);
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
