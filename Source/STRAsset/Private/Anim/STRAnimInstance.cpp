#include "Anim/STRAnimInstance.h"

USTRAnimInstance::USTRAnimInstance()
{
    SpriteAnim = nullptr;
    SpriteAnimValid = false;
    SpriteAnimTime = 0;
}

void USTRAnimInstance::SetAnimAndFrame(class UAnimSequence* InAnim, int32 InFrame)
{
    SpriteAnim = InAnim;
    SpriteAnimValid = InAnim != nullptr;
    SpriteAnimTime = float(InFrame) / 60;
}
