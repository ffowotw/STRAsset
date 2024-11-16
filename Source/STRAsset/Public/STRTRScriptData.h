#pragma once

#include "CoreMinimal.h"
#include "STRTRScriptData.generated.h"

// Subroutine type
UENUM(BlueprintType)
enum class STRSubroutineType : uint8
{
	STATE = 0,
	FUNCTION = 1 << 0,
};

// Subroutine Value
USTRUCT(BlueprintType)
struct FSTRSubroutineValue
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Header;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Value;
};

// Subroutine
USTRUCT(BlueprintType)
struct FSTRSubroutine
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    STRSubroutineType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)    
    TArray<FSTRSubroutineValue> SubroutineValues;
};

// Script data
UCLASS(BlueprintType)
class STRASSET_API USTRTRScriptData : public UObject
{
    GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FSTRSubroutine> Subroutines;

public:
    void SetScriptText(FString InString);

    UFUNCTION(BlueprintCallable, CallInEditor)
    void GenerateSubroutines();

private:
	UPROPERTY()
    FString m_scriptText;
};
