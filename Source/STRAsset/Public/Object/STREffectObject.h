#pragma once

#include "CoreMinimal.h"
#include "Object/STRObject.h"
#include "STREffectObject.generated.h"

UCLASS(BlueprintType)
class USTREffectObject : public USTRObject
{
    GENERATED_BODY()

public:
    void Init(ASTRGameMode* InGameMode, USTRObject* InParent, FString InObjectName, FSTRDataSet InDataSet);
    
    virtual void LateTicking() override;

public:
    bool CanHit() { return m_numberOfHits > 0; }
    void DecreaseHitNum() { m_numberOfHits--; }

    virtual bool CheckRequestedDestroy() override
    {
        if (m_destroyRequested || (m_parent && m_parent->CheckRequestedDestroy()))
        {
            OnDestroy();

            return true;
        }

        return false;
    }

protected:
    virtual class USTRScriptData* GetScriptData() override { return m_dataSet.EffectScriptData; }
    virtual class USTRCollisionData* GetCollisionData() override { return m_dataSet.CollisionData; }

protected:
    virtual void OnDamageOrGuard() override;
    virtual void OnDestroy() override;

private:
    virtual bool StateExecutions(FString InExecutionHeaderName, TArray<FString> InValues) override;

private:
    class USTRChara* FindCharaInParent()
    {
        if (!m_parent)
        {
            return nullptr;
        }

        if (m_parent->IsA(USTRChara::StaticClass()))
        {
            return Cast<USTRChara>(m_parent);
        }
        else if (m_parent->IsA(USTREffectObject::StaticClass()))
        {
            return Cast<USTREffectObject>(m_parent)->FindCharaInParent();
        }

        return nullptr;
    }

private:
    UPROPERTY()
    USTRObject* m_parent;

private:
    UPROPERTY()
    FString m_attackType = "NONE";

    int32 m_disableFlag;

    int32 m_numberOfHits = 0;

    bool m_destroyRequested;
    bool m_destroyOnPlayerStateChanged;
    bool m_destroyOnDamageCollision;
    bool m_destroyOnEnemyDamage;
    bool m_destroyOnEnemyGuard;
};
