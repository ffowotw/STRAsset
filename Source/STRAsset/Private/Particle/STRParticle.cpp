#include "Particle/STRParticle.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ASTRParticle::ASTRParticle()
{
    m_root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = m_root;
}

void ASTRParticle::Init(class UNiagaraSystem* InNiagaraSystem)
{
    m_particle = UNiagaraFunctionLibrary::SpawnSystemAttached(InNiagaraSystem, m_root, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::Type::KeepRelativeOffset, true);
    
    m_particle->OnSystemFinished.AddDynamic(this, &ASTRParticle::OnParticleEnded);
}

void ASTRParticle::LinkParticle(class USTRObject* InParent)
{
    m_parent = InParent;
}

void ASTRParticle::Render(int32 InFacing, int32 InPositionX, int32 InPositionY)
{
    SetActorScale3D(FVector(InFacing, 1, 1));
    SetActorLocation(FVector(float(InPositionX) / 1000, 0, float(InPositionY) / 1000));
}

void ASTRParticle::OnParticleEnded(class UNiagaraComponent* InNiagaraComponent)
{
    if (m_parent == nullptr)
    {
        Destroy();
    }
}
