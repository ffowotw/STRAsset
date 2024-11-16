#pragma once

#include "CoreMinimal.h"
#include "STRInput.h"
#include "STRInputBuffer.generated.h"

UCLASS()
class USTRInputBuffer : public UObject
{
    GENERATED_BODY()

public:
    void AddInput(int8 InDirection);
    void GetInput(uint32 InIndex, int8 InFacing, int8& OutDirection, uint32& OutFrameCount);

    static void GetDirectionFromAxis(int8 InHorizontal, int8 InVertical, int8& OutDirection);
    static void GetAxisFromDirection(int8 InDirection, int8& OutHorizontal, int8& OutVertical);

    int32 Num()
    {
        return m_buffer.Num();
    }

    void PrintBuffer()
    {
        UE_LOG(LogTemp, Warning, TEXT(" "));
        UE_LOG(LogTemp, Warning, TEXT(" "));
        UE_LOG(LogTemp, Warning, TEXT("---- START PRINTING BUFFER ----"));
        UE_LOG(LogTemp, Warning, TEXT(" "));
        
        for (FSTRInput input : m_buffer)
        {
            if (input.Direction == 0 || input.FrameCount == 0)
            {
                continue;
            }

            UE_LOG(LogTemp, Warning, TEXT("DIR: %i, FC: %i"), input.Direction, input.FrameCount);
        }

        UE_LOG(LogTemp, Warning, TEXT(" "));
        UE_LOG(LogTemp, Warning, TEXT("---- END PRINTING BUFFER ----"));
        UE_LOG(LogTemp, Warning, TEXT(" "));
        UE_LOG(LogTemp, Warning, TEXT(" "));
    }

private:
    int32 GetSize()
    {
        return sizeof(m_buffer) / sizeof(m_buffer[0]);
    }

private:
    UPROPERTY()
    TArray<FSTRInput> m_buffer;
};
