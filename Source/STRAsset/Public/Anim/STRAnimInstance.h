#pragma once

#include "CoreMinimal.h"
#include "STRAnimInstance.generated.h"

UCLASS(transient, Blueprintable, HideCategories=AnimInstance, BlueprintType)
class USTRAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USTRAnimInstance();

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    class UAnimSequence* CurrentAnim;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 CurrentFrame;

    void SetAnimAndFrame(class UAnimSequence* InAnim, int32 InFrame);
};
