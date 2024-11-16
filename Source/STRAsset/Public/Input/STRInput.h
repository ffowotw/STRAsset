#pragma once

#include "CoreMinimal.h"
#include "STRInput.generated.h"

USTRUCT()
struct FSTRInput
{
    GENERATED_BODY()

public:
	UPROPERTY()
    int8 Direction;
	UPROPERTY()
    uint32 FrameCount;
};
