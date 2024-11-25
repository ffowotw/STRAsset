#pragma once

#include "CoreMinimal.h"
#include "STRMeshSet.h"
#include "STRMove.h"
#include "Object/STRObject.h"
#include "STRChara.generated.h"

UCLASS(BlueprintType)
class USTRChara : public USTRObject
{
    GENERATED_BODY()

public:
    void PreInit(class ASTRGameMode* InGameMode, int32 InIndex, FString InLayer, class ASTRObjectRenderer* InRenderer, FSTRDataSet InDataSet);
    void Init();
    
public:
    // Tick
    void TickFacing()
    {
        if ((m_charaState == "JUMPING" && !m_highJumped) || m_moves.Contains(m_currentStateName) || CheckCurrentStateName("CmnActFDash"))
        {
            return;
        }

        CheckFacing();
    }
    void InputTicking();

    virtual void EarlyTicking() override;
    virtual void LateTicking() override;

    void TickInputCheck();
    void TickCommonActionCheck(bool InMoveEnded = false);
    
    void TickPushboxCheck();

    virtual void TickRender() override;

    virtual void TickDrawCollisions() override;

public:
    FString GetCharaName() { return m_charaName; }
    FString GetCharaState() { return m_charaState; }

    void CheckFacing();
    int32 GetPushboxWidth() { return m_pushboxWidth / 2; }

    bool IsStrikeInvul() { return m_strikeInvul != 0; }
    bool IsThrowInvul() { return m_throwInvul != 0; }
    bool IsProjectileInvul() { return m_projectileInvul != 0; }

    FString GetCounterState()
    {
        if (m_state == "STARTUP_STATE" || m_state == "ACTIVE_STATE")
        {
            return "COUNTER_STATE";
        }
        else if (m_state == "RECOVERY_STATE")
        {
            return "PUNISH_COUNTER_STATE";
        }

        return "NONE";
    }
    void ApplyHitEffect(FString InHitEffect)
    {
        //
    }
    void JumpToHitState(int32 InAttackLevel, USTRChara* InChara)
    {
        m_inHitStun = true;
        m_hitByChara = InChara;

        if (m_charaState == "STANDING")
        {
            JumpToState("CmnActHitHighLv" + FString::FromInt(InAttackLevel));
        }
        if (m_charaState == "CROUCHING")
        {
            JumpToState("CmnActHitCrouchLv" + FString::FromInt(InAttackLevel));
        }
        // else
        // {
        //     JumpToState("" + InAttackLevel);
        // }
    }

    void AddDisableFlag(int32 InFlag)
    {
        if (!m_disableFlags.Contains(InFlag))
        {
            m_disableFlags.Add(InFlag);
        }
    }
    void RemoveDisableFlag(int32 InFlag)
    {
        if (m_disableFlags.Contains(InFlag))
        {
            m_disableFlags.Remove(InFlag);
        }
    }

public:
    // Inputs
    void PlayerUp(bool InPressed);
    void PlayerDown(bool InPressed);
    void PlayerRight(bool InPressed);
    void PlayerLeft(bool InPressed);

    void PlayerButton(uint8 InButton, bool InPressed);


private:
    void UpdatePlayerInputs();
    void UpdateInputBuffer();

    int32 GetInputBufferLength()
    {
        return m_inputBuffer_directions.Num();
    }
    void AddInputToBuffer(int8 InDirection)
    {
        if (m_inputBuffer_directions.Num() == 0 || m_inputBuffer_directions[GetInputBufferLength() - 1] != InDirection)
        {
            m_inputBuffer_directions.Add(InDirection);
            m_inputBuffer_frameCounts.Add(1);
        }
        else
        {
            m_inputBuffer_frameCounts[GetInputBufferLength() - 1]++;
        }
    }
    void GetInputFromBuffer(int32 InIndex, int8 InFacing, int8& OutDirection, uint32& OutFrameCount)
    {
        if (InIndex < 0 || InIndex >= GetInputBufferLength())
        {
            OutDirection = -1;
            OutFrameCount = -1;

            return;
        }

        int8 direction;
        int8 horizontal, vertical;

        GetAxisFromDirection(m_inputBuffer_directions[InIndex], horizontal, vertical);
        GetDirectionFromAxis(horizontal * InFacing, vertical, direction);

        OutDirection = direction;
        OutFrameCount = m_inputBuffer_frameCounts[InIndex];
    }

    void GetDirectionFromAxis(int8 InHorizontal, int8 InVertical, int8& OutDirection)
    {
        if (InVertical > 1 || InVertical < -1 || InHorizontal > 1 || InHorizontal < -1)
        {
            return;
        }

        OutDirection = 5 + InVertical * 3 + InHorizontal;
    }
    void GetAxisFromDirection(int8 InDirection, int8& OutHorizontal, int8& OutVertical)
    {
        if (InDirection < 0 || InDirection > 9)
        {
            return;
        }

        OutHorizontal = ((InDirection - 1) % 3) - 1;
        OutVertical = (InDirection - 5 - OutHorizontal) / 3;
    }

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


    // Chara
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

    void SwapMeshSet(FString InMeshSet)
    {
        m_currentMeshSet = InMeshSet;
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

    // Executions
    virtual bool Execute(FString InExecutionHeaderName, TArray<FString> InValues) override;
    virtual bool FunctionExecutions(FString InExecutionHeaderName, TArray<FString> InValues) override;
    virtual bool StateExecutions(FString InExecutionHeaderName, TArray<FString> InValues) override;

private:
    virtual void ResetStateValues() override;
    virtual void ExitState() override
    {
        TickCommonActionCheck(true);
    }

private:
    // Getters
    FSTRCollision GetPushbox ()
    {
        return {
            "PUSHBOX",
            m_positionX - (m_pushboxWidth / 2 * m_facing),
            m_positionY - m_pushboxHeightLow,
            m_pushboxWidth * m_facing,
            m_pushboxHeight
        };
    };
    

    bool CheckIsFollowupMove(FString InString)
    {
        if (!m_moves.Contains(InString)) return false;

        return m_moves[InString].IsFollowupMove;
    }

private:
    // Detail
    int32 m_charaIndex;
    UPROPERTY()
    FString m_charaName;

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

    // Mesh set
    UPROPERTY()
    FString m_editingMeshSet;
    UPROPERTY()
    TMap<FString, FSTRMeshSet> m_meshSets;

    // Voice

private:
    // In Game Data
    UPROPERTY()
    FString m_charaState = "STANDING";

    TArray<int32> m_disableFlags;

    int32 m_pushboxWidth;
    int32 m_pushboxHeight;
    int32 m_pushboxHeightLow;

    bool m_highJumped;

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

    // Hit
    bool m_inHitStun;

    USTRChara* m_hitByChara;

    // Input
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
    TArray<int8> m_inputBuffer_directions;
	UPROPERTY()
    TArray<uint32> m_inputBuffer_frameCounts;

    // Animation
    UPROPERTY()
    FString m_currentAnimation;

    UPROPERTY()
    FString m_defaultMeshSet;
    UPROPERTY()
    FString m_currentMeshSet;
    UPROPERTY()
    TArray<FString> m_meshesNotDisplay;
    
    // Invuls
    int32 m_strikeInvul = 0;
    int32 m_throwInvul = 0;
    int32 m_projectileInvul = 0;
    
    // Physics
    int32 m_groundedX;
    int32 m_inertiaPercent = 100;

    // Gravity
    int32 m_gravityPercent = 100;

    // Collision
    bool m_noCollision;

    // Cancels
    UPROPERTY()
    TArray<FString> m_whiffCancels;
    UPROPERTY()
    TArray<FString> m_hitCancels;
};
