#include "Input/STRInputBuffer.h"

void USTRInputBuffer::AddInput(int8 InDirection)
{
    // UE_LOG(LogTemp, Warning, TEXT("%i"), InDirection);

    if (m_buffer.Num() == 0 || m_buffer[m_buffer.Num() - 1].Direction != InDirection)
    {
        m_buffer.Add({InDirection, 1});
    }
    else
    {
        m_buffer[m_buffer.Num() - 1].FrameCount++;
    }

    // UE_LOG(LogTemp, Warning, TEXT("DIR: %i, FC: %i"), m_buffer[m_buffer.Num() - 1].Direction, m_buffer[m_buffer.Num() - 1].FrameCount);
}

void USTRInputBuffer::GetInput(uint32 InIndex, int8 InFacing, int8& OutDirection, uint32& OutFrameCount)
{
    uint32 bufferLength = m_buffer.Num();

    if (InIndex >= bufferLength)
    {
        OutDirection = -1;
        OutFrameCount = -1;

        return;
    }

    int8 direction;
    int8 horizontal, vertical;

    GetAxisFromDirection(m_buffer[InIndex].Direction, horizontal, vertical);
    GetDirectionFromAxis(horizontal * InFacing, vertical, direction);

    OutDirection = direction;
    OutFrameCount = m_buffer[InIndex].FrameCount;
}

void USTRInputBuffer::GetDirectionFromAxis(int8 InHorizontal, int8 InVertical, int8& OutDirection)
{
    if (InVertical > 1 || InVertical < -1 || InHorizontal > 1 || InHorizontal < -1)
    {
        return;
    }

    OutDirection = 5 + InVertical * 3 + InHorizontal;
}

void USTRInputBuffer::GetAxisFromDirection(int8 InDirection, int8& OutHorizontal, int8& OutVertical)
{
    if (InDirection < 0 || InDirection > 9)
    {
        return;
    }

    OutHorizontal = ((InDirection - 1) % 3) - 1;
    OutVertical = (InDirection - 5 - OutHorizontal) / 3;
}
