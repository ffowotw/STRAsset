#pragma once

#include "CoreMinimal.h"
#include "STRTransform.generated.h"

USTRUCT()
struct FSTRTransform
{
	GENERATED_BODY()

public:
    FVector GetPosition()
    {
        return FVector(PositionX / 1000.0, PositionY / 1000.0, PositionZ / 1000.0);
    }
    FRotator GetRotation()
    {
        return FRotator(RotationX / 180.0, RotationY / 180.0, RotationZ / 180.0);
    }
	
public:
	UPROPERTY()
	int32 PositionX;
	UPROPERTY()
	int32 PositionY;
	UPROPERTY()
	int32 PositionZ;

	UPROPERTY()
	int32 RotationX;
	UPROPERTY()
	int32 RotationY;
	UPROPERTY()
	int32 RotationZ;
};
