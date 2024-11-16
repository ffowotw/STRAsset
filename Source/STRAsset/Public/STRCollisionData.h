#pragma once

#include "CoreMinimal.h"
#include "STRTRScriptData.h"
#include "STRCollisionData.generated.h"

// Collision
USTRUCT(BlueprintType)
struct FSTRCollision
{
    GENERATED_BODY()

public:
    static bool CheckCollide(struct FSTRCollision InCollisionA, struct FSTRCollision InCollisionB);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)    
    int32 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)    
    int32 Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)    
    int32 Width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)    
    int32 Height;
    
};

// Collisions
USTRUCT(BlueprintType)
struct FSTRCollisions
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSTRCollision> Collisions;
};

UCLASS(BlueprintType)
class STRASSET_API USTRCollisionData : public UObject
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FSTRCollisions> Collisions;

public:
    void SetScriptText(FString InString);

    UFUNCTION(BlueprintCallable, CallInEditor)
    void GenerateCollisions();

    TArray<FSTRCollision> GetCollisions(FString InType, FString InSpriteName, int32 InPositionX, int32 InPositionY, int32 InFacing);
    
private:
	UPROPERTY()
    FString m_scriptText;

};
