#pragma once

#include "CoreMinimal.h"
#include "STRDataSet.h"
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
        OutPositionX = m_positionX;
        OutPositionY = m_positionY;
    }
    void SetPosition(int32 InPositionX, int32 InPositionY)
    {
        m_positionX = InPositionX;
        m_positionY = InPositionY;

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
        return GetCollisionData()->GetCollisions(InType, m_currentSprite, m_positionX, m_positionY, m_facing);
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
        m_attackLevel = InAttackLevel;

        switch (InAttackLevel)
        {
        case 0:
        {
            m_hitStop = 11;

            m_hitPushbackX = 3000;
            m_hitPushbackY = 0;
            m_hitAirPushbackX = 0;
            m_hitAirPushbackY = 3000;
            m_counterHitAirPushbackX = 3000;
            m_counterHitAirPushbackY = 0;

            break;
        }
        case 1:
        {
            m_hitStop = 12;
            
            m_hitPushbackX = 3500;
            m_hitPushbackY = 0;
            m_hitAirPushbackX = 3500;
            m_hitAirPushbackY = 0;
            m_counterHitAirPushbackX = 3500;
            m_counterHitAirPushbackY = 0;

            break;
        }
        case 2:
        {
            m_hitStop = 13;
            
            m_hitPushbackX = 4000;
            m_hitPushbackY = 0;
            m_hitAirPushbackX = 4000;
            m_hitAirPushbackY = 0;
            m_counterHitAirPushbackX = 4000;
            m_counterHitAirPushbackY = 0;

            break;
        }
        case 3:
        {
            m_hitStop = 14;
            
            m_hitPushbackX = 4500;
            m_hitPushbackY = 0;
            m_hitAirPushbackX = 4500;
            m_hitAirPushbackY = 0;
            m_counterHitAirPushbackX = 4500;
            m_counterHitAirPushbackY = 0;

            break;
        }
        case 4:
        {
            m_hitStop = 15;
            
            m_hitPushbackX = 5000;
            m_hitPushbackY = 0;
            m_hitAirPushbackX = 5000;
            m_hitAirPushbackY = 0;
            m_counterHitAirPushbackX = 5000;
            m_counterHitAirPushbackY = 0;

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
    TArray<bool> m_passedIfStatement;
    bool m_canExecute;

    int32 m_IfStatementOperated;

protected:
    // State
    UPROPERTY()
    FString m_state = "NONE";

    int32 m_freezeTime;
    int8 m_facing;

    // Physics
    int32 m_positionX;
    int32 m_positionY;

    int32 m_velocityX;
    int32 m_velocityY;

    int32 m_velocityXPercent;
    int32 m_velocityYPercent;
    int32 m_velocityXPercentEachFrame;
    int32 m_velocityYPercentEachFrame;
    
    // Gravity
    int32 m_gravity = 0;

    // Animation
    bool m_stepFrame;

    UPROPERTY()
    FString m_currentSprite;

    // Throw
    bool m_isThrow;
    bool m_canThrowHitStun;
    int32 m_throwRange;
    UPROPERTY()
    FString m_executeOnHit;

    int32 m_enemyGrabSprite;

    // Attack
    int32 m_damage;
    int32 m_minDamagePercent;
    int32 m_proration;

    int32 m_attackLevel;
    int32 m_attackAngle;
    bool m_isAirUnblockable;

    bool m_noPushbackScaling;
    bool m_noHitstunScaling;
    bool m_noGravityScaling;

    FString m_guardType;
    FString m_counterHitType;

    int32 m_hitStop;
    bool m_shortHitStop;
    bool m_disableHitStop;

    // Hit
    int32 m_hitGravity;

    int32 m_hitPushbackX;
    int32 m_hitPushbackY;

    int32 m_hitAirPushbackX;
    int32 m_hitAirPushbackY;

    int32 m_counterHitAirPushbackX;
    int32 m_counterHitAirPushbackY;

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

    // Roll
    int32 m_rollCount;
    int32 m_rollDuration;
    int32 m_counterHitRollDuration;

    // Wall Stick
    int32 m_wallStickDuration;
    int32 m_counterHitWallStickDuration;
    
    // Ground Bounce
    int32 m_groundBounceCount;
    int32 m_groundBounceYVelocityPercent;

    int32 m_counterHitGroundBounceCount;
    int32 m_counterHitGroundBounceYVelocityPercent;

    // Wal Bounce
    bool m_wallBounceInCornerOnly;
    bool m_counterHitWallBounceInCornerOnly;

    int32 m_wallBounceCount;
    int32 m_counterHitWallBounceCount;

    int32 m_wallBounceXVelocityPercent;
    int32 m_counterHitWallBounceXVelocityPercent;
};
