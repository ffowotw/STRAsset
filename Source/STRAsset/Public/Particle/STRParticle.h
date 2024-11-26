#pragma once

#include "CoreMinimal.h"
#include "STRParticle.generated.h"

UCLASS(BlueprintType)
class ASTRParticle : public AActor
{
    GENERATED_BODY()

public:
    ASTRParticle();
    
public:
    void Init(class UNiagaraSystem* InNiagaraSystem);
    void LinkParticle(class USTRObject* InParent);

    void Render(int32 InFacing, int32 InPositionX, int32 InPositionY);

    UFUNCTION()
    void OnParticleEnded(class UNiagaraComponent* InNiagaraComponent);

private:
    UPROPERTY()
    USceneComponent* m_root;

    UPROPERTY()
    class USTRObject* m_parent;

    UPROPERTY()
    class UNiagaraComponent* m_particle;
};
