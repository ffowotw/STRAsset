#include "Anim/STRAnimInstance.h"

USTRAnimInstance::USTRAnimInstance()
{
    CurrentAnim = nullptr;
    CurrentFrame = 0;
}

void USTRAnimInstance::SetAnimAndFrame(class UAnimSequence* InAnim, int32 InFrame)
{
    CurrentAnim = InAnim;
    CurrentFrame = InFrame;
}
