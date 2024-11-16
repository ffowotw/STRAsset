#pragma once

#include "CoreMinimal.h"
#include "STRMove.h"
#include "STRMeshSet.h"
#include "STRCollisionData.h"
#include "STRMeshArray.h"
#include "STRCharaSet.h"
#include "Input/STRInputBuffer.h"
#include "STRChara.generated.h"

UCLASS(BlueprintType)
class USTRChara : public UObject
{
    GENERATED_BODY()

public:
    void PreInit(class ASTRGameMode* InGameMode, int32 InIndex, FString InLayer, class ASTRCharaRenderer* InRenderer, FSTRCharaSet InCharaSet);
    void Init();
    
public:
    #pragma region Functions - Tick

    void InputTicking();
    void Ticking();
    void ContinuousTicking();

    void TickFacing();
    void TickInputCheck();
    void TickMoving();
    
    void TickPushboxCheck();

    void TickHitCheck();
    void TickDamageCheck();

    void TickRender();

    void TickDrawCollisions();

    #pragma endregion Functions - Tick


    #pragma region Functions - Data

    FString GetCharaName() { return m_charaName; }

    FString GetLayer() { return m_charaLayer; }

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

    void QueueDamage();

    #pragma endregion Functions - Data


    #pragma region Functions - Player

    void PlayerUp(bool InPressed);
    void PlayerDown(bool InPressed);
    void PlayerRight(bool InPressed);
    void PlayerLeft(bool InPressed);

    void PlayerButton(uint8 InButton, bool InPressed);

    #pragma endregion Functions - Player

private:
    #pragma region Functions - Inputs

    void UpdatePlayerInputs();
    void UpdateInputBuffer();

    void ClearButtonBuffer()
    {
        for (int32 i = 0; i < m_buttonBuffer.Num(); i++)
        {
            m_buttonBuffer[i] = 0;
        }
    };

    bool CheckMoveInput(FSTRMove InMove);
    bool CheckInput(FString InInput);
    bool CheckDirection(FString InDirection);
    bool MatchDirection(FString InDirection, int8 InHorizontal, int8 InVertical);

    #pragma endregion Functions - Inputs


    #pragma region Functions - Chara

    void SetCharaState(FString InState)
    {
        if (InState == "STANDING")
        {
            m_pushboxWidth = m_pushboxWidthStand;
            m_pushboxHeight = m_pushboxHeightStand;
            m_pushboxHeightLow = 0;
        }
        else if (InState == "CROUCHING")
        {
            m_pushboxWidth = m_pushboxWidthCrouch;
            m_pushboxHeight = m_pushboxHeightCrouch;
            m_pushboxHeightLow = 0;
        }
        else if (InState == "JUMPING")
        {
            m_pushboxWidth = m_pushboxWidthAir;
            m_pushboxHeight = m_pushboxHeightAir;
            m_pushboxHeightLow = m_pushboxHeightLowAir;
        }
        else
        {
            return;
        }

        m_charaState = InState;
    }

    void RestoreAirJump()
    {
        m_airJumpCount = m_maxAirJumpCount;
        m_enableJump = true;
    }

    void RestoreAirDash()
    {
        m_airDashCount = m_maxAirDashCount;
    }

    #pragma endregion Functions - Chara


    #pragma region Functions - Executions
    
    void Execute(FString InExecutionHeaderName, TArray<FString> InValues);
    bool FunctionExecutions(FString InExecutionHeaderName, TArray<FString> InValues);
    void StateExecutions(FString InExecutionHeaderName, TArray<FString> InValues);

    #pragma endregion Functions - Executions

private:
    void JumpToState(FString InStateName);
    void ResetStateValues();
    void StateExecution();
    void GotoLabel(FString InLabelName);

    void ExitState()
    {
        if (m_moves.Contains(m_currentStateName))
        {
            m_currentStateName = "";
            TickMoving();
        }
        else
        {
            JumpToState("");
        }
    }

private:
    void CallFunction(FString InFunctionName);

private:
    #pragma region Functions - Getters
    
    FString GetString(FString InString)
    {
        int32 startIndex = InString.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, 0);
        int32 endIndex = InString.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, startIndex + 1);
        
        return InString.Mid(startIndex + 1, endIndex - startIndex - 1);
    };

    FString GetEnum(FString InString)
    {
        int32 startIndex = InString.Find("(", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, 0);
        int32 endIndex = InString.Find(")", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, startIndex + 1);
        
        return InString.Mid(startIndex + 1, endIndex - startIndex - 1);
    };

    int32 GetInt(FString InString)
    {
        return FCString::Atoi(*InString);
    };

    bool GetBool(FString InString);
    bool GetBool(TArray<FString> InArray);


    FSTRCollision GetPushbox ()
    {
        return {
            "PUSHBOX",
            m_positionX,
            m_positionY + (m_pushboxHeight - m_pushboxHeightLow) / 2,
            m_pushboxWidth / 2,
            (m_pushboxHeight + m_pushboxHeightLow) / 2
        };
    };

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
        return m_collisionData->GetCollisions(InType, m_currentSprite, m_positionX, m_positionY, m_facing);
    }

    #pragma endregion Functions - Getters


    #pragma region Functions - Checkings

    bool CheckCurrentStateName(FString InString)
    {
        return m_currentStateName == InString;
    };

    bool CheckLastStateName(FString InString)
    {
        return m_lastStateName == InString;
    };

    bool CheckIsFollowupMove(FString InString)
    {
        if (!m_moves.Contains(InString)) return false;

        return m_moves[InString].IsFollowupMove;
    }

    #pragma endregion Functions - Checkings

private:
    #pragma region Parameters - Data

    UPROPERTY()
    class ASTRGameMode* m_gameMode;
    
    UPROPERTY()
    class ASTRCharaRenderer* m_renderer;

    UPROPERTY()
    class USTRTRScriptData* m_scriptData;
    UPROPERTY()
    class USTRCollisionData* m_collisionData;

    // Detail
    int32 m_charaIndex;
    UPROPERTY()
    FString m_charaName;
    UPROPERTY()
    FString m_charaLayer;

    int32 m_weight;
	int32 m_defence;
	int32 m_maxAirJumpCount;
	int32 m_maxAirDashCount;

    // Walk
	int32 m_walkFSpeed;
	int32 m_walkBSpeed;

    // Dash
	int32 m_dashFInitSpeed;
	int32 m_dashFAcceleration;
	int32 m_dashFriction;
	int32 m_dashBXSpeed;
	int32 m_dashBYSpeed;
	int32 m_dashBGravity;

    // Jump
	int32 m_jumpFXSpeed;
	int32 m_jumpBXSpeed;
	int32 m_jumpYSpeed;
	int32 m_jumpGravity;

    // High jump
	int32 m_highJumpFXSpeed;
	int32 m_highJumpBXSpeed;
	int32 m_highJumpYSpeed;
	int32 m_highJumpGravity;

    // Air dash
	int32 m_airDashMinHeight;
	int32 m_airDashFTime;
	int32 m_airDashBTime;
	int32 m_airDashFSpeed;
	int32 m_airDashBSpeed;
	int32 m_airDashFNoAttackTime;
	int32 m_airDashBNoAttackTime;

    // Push box
	int32 m_pushboxWidthStand;
	int32 m_pushboxHeightStand;
	int32 m_pushboxWidthCrouch;
	int32 m_pushboxHeightCrouch;
	int32 m_pushboxWidthAir;
	int32 m_pushboxHeightAir;
	int32 m_pushboxHeightLowAir;

    // Moves
    UPROPERTY()
    FString m_editingMove;
    UPROPERTY()
    TArray<FString> m_moveKeys;
    UPROPERTY()
    TMap<FString, FSTRMove> m_moves;

    // Damage sprites
    UPROPERTY()
    TMap<int32, FString> m_damageSprites;
    UPROPERTY()
    TMap<FString, FString> m_damageSpritesEx;

    // Mesh set
    UPROPERTY()
    FString m_editingMeshSet;
    UPROPERTY()
    TMap<FString, FSTRMeshSet> m_meshSets;

    // Voice

    // Stats Skill Check
    UPROPERTY()
    TMap<int32, FString> m_statsSkillChecks;

    #pragma endregion Parameters - Data


    #pragma region Parameters - In Game Data

    UPROPERTY()
    FString m_currentStateName;
    UPROPERTY()
    FString m_lastStateName;
    UPROPERTY()
    TMap<FString, int32> m_labels;

    int32 m_stateExecutionIndex;
    int32 m_stateExecutionCountdown;
    int32 m_stateExecutedFrameCount;

    UPROPERTY()
    FString m_charaState = "STANDING";
    UPROPERTY()
    FString m_charaStatement = "NONE";

    UPROPERTY()
    FString m_currentMeshSet;
    UPROPERTY()
    TArray<FString> m_meshesNotDisplay;
    
    int32 m_strikeInvul;
    int32 m_throwInvul;
    bool m_noCollision;
    
    UPROPERTY()
    TMap<FString, int32> m_storedVal;

    int8 m_facing = 1;

    int32 m_positionX;
    int32 m_positionY;

    int32 m_velocityX;
    int32 m_velocityY;

    int32 m_gravity = 0;

    int32 m_pushboxWidth;
    int32 m_pushboxHeight;
    int32 m_pushboxHeightLow;

    bool m_pushChecked;
    bool m_highJumped;
    int32 m_groundedX;

	int32 m_airJumpCount;

	int32 m_airDashCount;
    int32 m_airDashTime;
    int32 m_airDashNoAttackTime;

    bool m_enableJump;
    bool m_enableNormals;
    bool m_enableSpecials;
    bool m_enableJumpCancel;
    bool m_enableWhiffCancel;
    bool m_enableSpecialCancel;

    UPROPERTY()
    TArray<USTRChara*> m_hitCharaList;
    // UPROPERTY()
    // TArray<int32> m_queuedDamage;

    #pragma endregion Parameters - In Game Data


    #pragma region Parameters - Input

    bool m_playerUp;
    bool m_playerDown;
    bool m_playerRight;
    bool m_playerLeft;

    int8 m_inputX;
    int8 m_inputY;

    UPROPERTY()
    TArray<uint8> m_queuedPressedButtons;
    UPROPERTY()
    TArray<uint8> m_queuedReleasedButtons;
    
    UPROPERTY()
    TArray<uint8> m_inputButtons;
    UPROPERTY()
    TArray<uint8> m_lastFrameInputButtons;
    UPROPERTY()
    TArray<uint8> m_buttonBuffer;

    UPROPERTY()
    USTRInputBuffer* m_inputBuffer;

    #pragma endregion Parameters - Input


    #pragma region Parameters - If Statement

    bool m_canExecute;

    int32 m_endIfPosition;
    int32 m_IfStatementOperated;

    #pragma endregion Parameters - If Statement


    #pragma region Parameters - Throw Details

    bool m_canGrab;
    int32 m_throwRange;

    #pragma endregion Parameters - Throw Details


    #pragma region Parameters - Move Details

    int32 m_damage;
    float m_proration;
    int32 m_attackLevel;
    int32 m_attackAngle;
    FString m_guardType;

    int32 m_hitStop;
    bool m_shortHitStop;
    bool m_disableHitStop;

    bool m_restoreAirJump;
    bool m_restoreAirDash;

    bool m_enabledJumpCancel;
    bool m_enabledSpecialCancel;

    UPROPERTY()
    TArray<FString> m_whiffLinkOptions;
    UPROPERTY()
    TArray<FString> m_hitLinkOptions;

    #pragma endregion Parameters - Move Details


    #pragma region Parameters - Animation

    UPROPERTY()
    FString m_currentSprite;

    #pragma endregion Parameters - Animation


    #pragma region Parameters - Physics Details

    int32 m_inertiaPercent;

    int32 m_velocityXPercent;
    int32 m_velocityYPercent;
    int32 m_velocityXPercentEachFrame;
    int32 m_velocityYPercentEachFrame;

    #pragma endregion Parameters - Physics Details


    #pragma region Parameters - Hit Effect Details
    
    UPROPERTY()
    FString m_groundHitEffect = "NORMAL_UPPER";
    UPROPERTY()
    FString m_airHitEffect = "NORMAL_UPPER";
    UPROPERTY()
    FString m_counterGroundHitEffect = "NORMAL_UPPER";
    UPROPERTY()
    FString m_counterAirHitEffect = "NORMAL_UPPER";

    #pragma endregion Parameters - Hit Effect Details


    #pragma region Parameters - Hit Details

    int32 m_hitGravity;

    int32 m_hitPushbackX;
    int32 m_hitPushbackY;

    int32 m_wallStickDuration;
    int32 m_rollDuration;

    int32 m_groundBounceCount;
    int32 m_groundBounceYVelocityPercent;

    int32 m_wallBounceCount;
    int32 m_wallBounceXVelocityPercent;
    bool m_wallBounceInCornerOnly;

    #pragma endregion Parameters - Hit Details


    #pragma region Parameters - Counter Hit Details

    int32 m_counterHitRollDuration;

    int32 m_counterHitGroundBounceCount;
    int32 m_counterHitGroundBounceYVelocityPercent;

    int32 m_counterHitWallBounceCount;
    int32 m_counterHitWallBounceXVelocityPercent;

    #pragma endregion Parameters - Counter Hit Details


    #pragma region Parameters - Hit Air Details

    int32 m_hitAirPushbackX;
    int32 m_hitAirPushbackY;

    #pragma endregion Parameters - Hit Air Details


    #pragma region Parameters - Counter Hit Air Details

    int32 m_counterHitAirPushbackX;
    int32 m_counterHitAirPushbackY;

    #pragma endregion Parameters - Counter Hit Air Details
};
