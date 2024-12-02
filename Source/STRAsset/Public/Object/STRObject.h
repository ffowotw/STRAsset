#pragma once

#include "CoreMinimal.h"
#include "Structures/STRDataSet.h"
#include "STRCollisionData.h"
#include "STRObject.generated.h"

UCLASS(BlueprintType)
class USTRObject : public UObject
{
    GENERATED_BODY()

public:
    virtual void EarlyTicking();
    void Ticking();
    virtual void LateTicking();
    
    void TickHitCheck();
    void TickDamageCheck();

    virtual void TickRender();

    virtual void TickDrawCollisions();

public:
    virtual bool CheckRequestedDestroy() { return false; }

public:
    FString GetLayer() { return m_layer; }

    void GetPosition(int32& OutPositionX, int32& OutPositionY)
    {
        OutPositionX = GetValue("200");
        OutPositionY = GetValue("201");
    }
    void SetPosition(int32 InPositionX, int32 InPositionY)
    {
        StoreValue("200", InPositionX);
        StoreValue("201", InPositionY);

        TickRender();
    }
    
    int8 GetFacing() { return m_facing; }

    bool Tickable() { return m_freezeTime == 0; }

    int32 GetFreezeTime() { return m_freezeTime; }
    void SetFreezeTime(int32 InFreezeTime)
    {
        if (InFreezeTime > m_freezeTime)
        {
            m_freezeTime = InFreezeTime;
        }
    }
    void DecreaseFreezeTime()
    { 
        if (m_freezeTime > 0)
        {
            m_freezeTime--;
        }
    };

    class ASTRObjectRenderer* GetRenderer() { return m_renderer; }

protected:
    virtual class USTRScriptData* GetScriptData() { return m_dataSet.ScriptData; }
    virtual class USTRCollisionData* GetCollisionData() { return m_dataSet.CollisionData; }

protected:
    virtual void OnDamageOrGuard() {};
    virtual void OnDestroy();

protected:
    // Execution
    virtual bool Execute(FString InExecutionHeaderName, TArray<FString> InValues);
    virtual bool FunctionExecutions(FString InExecutionHeaderName, TArray<FString> InValues);
    virtual bool StateExecutions(FString InExecutionHeaderName, TArray<FString> InValues);

protected:
    // State
    void JumpToState(FString InStateName);
    void StateExecution();
    void GotoLabel(FString InLabelName);
    virtual void ResetStateValues();
    virtual void ExitState()
    {
        JumpToState("");
    }

    bool CheckCurrentStateName(FString InString)
    {
        return m_currentStateName == InString;
    };
    bool CheckLastStateName(FString InString)
    {
        return m_lastStateName == InString;
    };

protected:
    // Function
    void CallFunction(FString InFunctionName);

protected:
    // Stored Value
    bool ContainsValue(FString InKey)
    {
        return m_storedVal.Contains(InKey);
    }

    void StoreValue(FString InKey, int32 InValue)
    {
        if (ContainsValue(InKey))
        {
            m_storedVal[InKey] = InValue;
        }
        else
        {
            m_storedVal.Add(InKey, InValue);
        }
    }

    int32 GetValue(FString InKey)
    {
        if (m_storedVal.Contains(InKey))
        {
            return m_storedVal[InKey];
        }

        return 0;
    }

    void CopyValue(FString InToKey, FString InFromKey)
    {
        if (ContainsValue(InFromKey))
        {
            StoreValue(InToKey, GetValue(InFromKey));
        }
    }

    void CopyValueFrom(USTRObject* InFromObject, FString InToKey, FString InFromKey)
    {
        if (InFromObject->ContainsValue(InFromKey))
        {
            StoreValue(InToKey, InFromObject->GetValue(InFromKey));
        }
    }

    void ModifyValue(FString InModifyKey, FString InTargetKey, int32 InValue)
    {
        if (!ContainsValue(InTargetKey))
        {
            return;
        }

        if (InModifyKey == "ADD")
        {
            StoreValue(InTargetKey, GetValue(InTargetKey) + InValue);

            return;
        }

        if (InModifyKey == "SUB")
        {
            StoreValue(InTargetKey, GetValue(InTargetKey) - InValue);

            return;
        }

        if (InModifyKey == "MUL")
        {
            StoreValue(InTargetKey, GetValue(InTargetKey) * InValue);

            return;
        }

        if (InModifyKey == "DIV")
        {
            StoreValue(InTargetKey, GetValue(InTargetKey) / InValue);

            return;
        }
    }

protected:
    // Getters
    FString GetString(FString InString)
    {
        int32 startIndex = InString.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, 0);
        int32 endIndex = InString.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, startIndex + 1);
        
        return InString.Mid(startIndex + 1, endIndex - startIndex - 1);
    }
    FString GetEnum(FString InString)
    {
        int32 startIndex = InString.Find("(", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, 0);
        int32 endIndex = InString.Find(")", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, startIndex + 1);
        
        return InString.Mid(startIndex + 1, endIndex - startIndex - 1);
    }
    int32 GetInt(FString InString)
    {
        return FCString::Atoi(*InString);
    }
    bool GetBool(FString InString)
    {
        if (InString.StartsWith("Val"))
        {
            FString key = InString.Mid(3);

            return m_storedVal[key] > 0 ? true : false;
        }

        return GetInt(InString) > 0 ? true : false;
    }
    bool GetBool(TArray<FString> InArray)
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

    TArray<FSTRCollision> GetHurtbox()
    {
        return GetCollisions("HURTBOX");
    }
    TArray<FSTRCollision> GetHitbox()
    {
        return GetCollisions("HITBOX");
    }
    TArray<FSTRCollision> GetCollisions(FString InType)
    {
        if (GetCollisionData())
        {
            return {};
        }
        
        return GetCollisionData()->GetCollisions(InType, m_currentSprite, GetValue("200"), GetValue("201"), m_facing);
    }

	int32 GetAdditionalHitStop(int32 InAttackLevel, bool InCounterHit)
	{
		switch (InAttackLevel)
		{
		case 0:
			return (InCounterHit ? 0 : 0);
		case 1:
			return (InCounterHit ? 2 : 0);
		case 2:
			return (InCounterHit ? 4 : 0);
		case 3:
			return (InCounterHit ? 8 : 0);
		case 4:
			return (InCounterHit ? 12 : 0);
		}

		return 0;
	}

protected:
    // Setters
    void SetAttackLevel(int32 InAttackLevel)
    {
        StoreValue("64", InAttackLevel);

        switch (InAttackLevel)
        {
        case 0:
        {
            StoreValue("68", 11);

            StoreValue("70", 3000);
            StoreValue("71", 0);
            StoreValue("72", 3000);
            StoreValue("73", 0);
            StoreValue("74", 3000);
            StoreValue("75", 0);

            break;
        }
        case 1:
        {
            StoreValue("68", 12);

            StoreValue("70", 3500);
            StoreValue("71", 0);
            StoreValue("72", 3500);
            StoreValue("73", 0);
            StoreValue("74", 3500);
            StoreValue("75", 0);

            break;
        }
        case 2:
        {
            StoreValue("68", 13);

            StoreValue("70", 4000);
            StoreValue("71", 0);
            StoreValue("72", 4000);
            StoreValue("73", 0);
            StoreValue("74", 4000);
            StoreValue("75", 0);

            break;
        }
        case 3:
        {
            StoreValue("68", 14);

            StoreValue("70", 4500);
            StoreValue("71", 0);
            StoreValue("72", 4500);
            StoreValue("73", 0);
            StoreValue("74", 4500);
            StoreValue("75", 0);

            break;
        }
        case 4:
        {
            StoreValue("68", 15);

            StoreValue("70", 5000);
            StoreValue("71", 0);
            StoreValue("72", 5000);
            StoreValue("73", 0);
            StoreValue("74", 5000);
            StoreValue("75", 0);

            break;
        }
        }
    }


protected:
    // Data
    UPROPERTY()
    class ASTRGameMode* m_gameMode;
    
    UPROPERTY()
    FSTRDataSet m_dataSet;

    UPROPERTY()
    class ASTRObjectRenderer* m_renderer;

    UPROPERTY()
    TArray<class ASTRParticle*> m_particles;

    int32 m_life;
    UPROPERTY()
    FString m_layer;

    // State
    UPROPERTY()
    FString m_currentStateName;
    UPROPERTY()
    FString m_lastStateName;
    UPROPERTY()
    TMap<FString, int32> m_labels;

    int32 m_stateExecutionIndex;
    int32 m_stateExecutionCountdown;

    UPROPERTY()
    TMap<FString, int32> m_storedVal;

    // If Statement
    int32 m_ifStatementLayer;
    UPROPERTY()
    TArray<bool> m_passedIfStatement;
    bool m_canExecute;

    int32 m_IfStatementOperated;

protected:
    // State
    UPROPERTY()
    FString m_state = "NONE";

    int32 m_freezeTime;
    int8 m_facing;

    // Animation
    UPROPERTY()
    FString m_animStatment;

    UPROPERTY()
    FString m_currentSprite;

    // Throw
    UPROPERTY()
    FString m_executeOnHit;

    // Attack
    bool m_noPushbackScaling;
    bool m_noHitstunScaling;
    bool m_noGravityScaling;

    UPROPERTY()
    FString m_guardType;
    UPROPERTY()
    FString m_counterHitType;

    UPROPERTY()
    TArray<USTRObject*> m_hitList;
    UPROPERTY()
    TArray<USTRObject*> m_queuedHit;

    // Hit Effects
    UPROPERTY()
    FString m_groundHitEffect = "NORMAL_UPPER";
    UPROPERTY()
    FString m_airHitEffect = "NORMAL_UPPER";
    UPROPERTY()
    FString m_groundCounterHitEffect = "NORMAL_UPPER";
    UPROPERTY()
    FString m_airCounterHitEffect = "NORMAL_UPPER";
};
