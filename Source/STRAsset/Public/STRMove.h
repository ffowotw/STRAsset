#pragma once

#include "CoreMinimal.h"
#include "STRMove.generated.h"

USTRUCT(BlueprintType)
struct FSTRMove
{
    GENERATED_BODY()

public:
    FString Type;
    FString CharaState;
    TArray<FString> MoveInputs;
    FString MoveRequirement;

    bool DisableMoveCanceling = false;
    bool IsFollowupMove = false;
};