#include "Object/STREffectObject.h"
#include "DataAssets/STRParticleDataAsset.h"
#include "Particle/STRParticle.h"
#include "Renderer/STREffectObjectRenderer.h"
#include "STRGameMode.h"

void USTREffectObject::Init(ASTRGameMode* InGameMode, USTRObject* InParent, FString InObjectName, FSTRDataSet InDataSet)
{
    m_gameMode = InGameMode;
    m_parent = InParent;

    CopyValueFrom(InParent, "200", "200");
    CopyValueFrom(InParent, "201", "201");

    m_facing = m_parent->GetFacing();
    m_layer = InParent->GetLayer();

    m_dataSet = InDataSet;

    JumpToState(InObjectName);
}

void USTREffectObject::LateTicking()
{
    if (abs(GetValue("200")) >= 1600000)
    {
        m_destroyRequested = true;
    }

    Super::LateTicking();
}

void USTREffectObject::OnDamageOrGuard()
{
    if (GetValue("134") == 1 || GetValue("135") == 1 || GetValue("136") == 1)
    {
        ModifyValue("SUB", "132", 1);

        if (GetValue("132") <= 0)
        {
            m_destroyRequested = true;
        }
    }
}

void USTREffectObject::OnDestroy()
{
    if (m_disableFlag > 0)
    {
        FindCharaInParent()->RemoveDisableFlag(m_disableFlag);
    }

    for (ASTRParticle* particle : m_particles)
    {
        particle->Destroy();
    }

    Super::OnDestroy();
}

bool USTREffectObject::StateExecutions(FString InExecutionHeader, TArray<FString> InValues)
{
    if (Super::StateExecutions(InExecutionHeader, InValues))
    {
        return true;
    }

    // Object
    if (InExecutionHeader == "deactivateObject")
    {
        return true;
    }

    // Disable Flag
    if (InExecutionHeader == "addDisableFlag")
    {
        m_disableFlag = GetInt(InValues[0]);

        FindCharaInParent()->AddDisableFlag(m_disableFlag);
        
        return true;
    }

    // Attack
    if (InExecutionHeader == "normalAttack")
    {
        m_attackType = "NORMAL";
    }
    if (InExecutionHeader == "specialAttack")
    {
        m_attackType = "SPECIAL";
    }
    if (InExecutionHeader == "ultimateAttack")
    {
        m_attackType = "ULTIMATE";
    }
    if (InExecutionHeader == "numberOfHits")
    {
        StoreValue("132", GetInt(InValues[0]));
    }

    // Targeting
    if (InExecutionHeader == "targetObject")
    {
        return true;
    }
    if (InExecutionHeader == "endTargetObject")
    {
        return true;
    }

    // Destroy
    if (InExecutionHeader == "requestDestroy")
    {
        m_destroyRequested = true;

        return true;
    }
    if (InExecutionHeader == "destroyOnPlayerStateChanged")
    {
        StoreValue("133", GetBool(InValues) ? 1 : 0);
        
        return true;
    }
    if (InExecutionHeader == "destroyOnDamageCollision")
    {
        StoreValue("134", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "destroyOnEnemyDamage")
    {
        StoreValue("135", GetBool(InValues) ? 1 : 0);

        return true;
    }
    if (InExecutionHeader == "destroyOnEnemyGuard")
    {
        StoreValue("136", GetBool(InValues) ? 1 : 0);
        
        return true;
    }
    
    // Link Object
    if (InExecutionHeader == "linkPositionToObject")
    {
        return true;
    }
    if (InExecutionHeader == "linkObjectZ")
    {
        return true;
    }
    if (InExecutionHeader == "linkObjectSize")
    {
        return true;
    }
    if (InExecutionHeader == "unlinkObject")
    {
        m_parent = nullptr;
    }

    // Link Destroy
    if (InExecutionHeader == "linkObjectDestroyOnStateChange")
    {
        return true;
    }
    if (InExecutionHeader == "linkObjectDestroyOnDamage")
    {
        return true;
    }

    // Particle
    if (InExecutionHeader == "linkParticle")
    {
        ASTRParticle* particle = m_gameMode->CreateParticle(m_dataSet.ParticleDataAsset, GetString(InValues[0]));

        if (particle != nullptr)
        {
            particle->LinkParticle(this);
            particle->Render(m_facing, GetValue("200"), GetValue("201"));

            m_particles.Add(particle);
        }

        return true;
    }


    return false;
}
