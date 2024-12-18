#include "STRCollisionData.h"

bool FSTRCollision::CheckCollide(FSTRCollision InCollisionA, FSTRCollision InCollisionB)
{
    int32 xA, xB, yA, yB;

    if (InCollisionA.X < InCollisionB.X)
    {
        xA = InCollisionA.X + InCollisionA.Width;
        xB = InCollisionB.X;
    }
    else
    {
        xA = InCollisionB.X + InCollisionB.Width;
        xB = InCollisionA.X;
    }

    if (InCollisionA.Y < InCollisionB.Y)
    {
        yA = InCollisionA.Y + InCollisionA.Height;
        yB = InCollisionB.Y;
    }
    else
    {
        yA = InCollisionB.Y + InCollisionB.Height;
        yB = InCollisionA.Y;
    }

    return xA > xB && yA > yB;
}

void USTRCollisionData::SetScriptText(FString InString)
{
    m_scriptText = InString;
}

void USTRCollisionData::GenerateCollisions()
{
    Collisions.Empty();

    TArray<FString> scriptLines;
    m_scriptText.ParseIntoArray(scriptLines, TEXT("\n"), true);

    FString currentCollisionSet = "";
    FString currentCollisions = "";
    
    for (FString str : scriptLines)
    {
        if (str == "")
        {
            continue;
        }

        FString header, value;

        str.TrimStartAndEnd().Replace(TEXT(" "), TEXT("")).Split(TEXT(":"), &header, &value);

        if (header == "")
        {
            continue;
        }

        if (currentCollisionSet == "" && header == "beginCollisionSet")
        {
            currentCollisionSet = value;

            int32 startIndex = currentCollisionSet.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, 0);
            int32 endIndex = currentCollisionSet.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, startIndex + 1);
            
            currentCollisionSet = currentCollisionSet.Mid(startIndex + 1, endIndex - startIndex - 1);
        }
        else if (header == "endCollisionSet")
        {
            currentCollisionSet = "";
        }
        else if (currentCollisions == "" && header == "beginCollisions")
        {
            currentCollisions = value;

            int32 startIndex = currentCollisions.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, 0);
            int32 endIndex = currentCollisions.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, startIndex + 1);
            
            currentCollisions = currentCollisionSet + "_" + currentCollisions.Mid(startIndex + 1, endIndex - startIndex - 1);
        
            Collisions.Add(currentCollisions, {});
        }
        else if (header == "endCollisions")
        {
            currentCollisions = "";
        }
        else
        {
            TArray<FString> values;

            value.ParseIntoArray(values, TEXT(","), true);

            Collisions[currentCollisions].Collisions.Add({
                header == "hit" ? "HITBOX" : "HURTBOX",
                FCString::Atoi(*values[0]) * 1000,
                FCString::Atoi(*values[1]) * 1000,
                FCString::Atoi(*values[2]) * 1000,
                FCString::Atoi(*values[3]) * 1000
            });
        }
    }
}

TArray<FSTRCollision> USTRCollisionData::GetCollisions(FString InType, FString InSpriteName, int32 InPositionX, int32 InPositionY, int32 InFacing)
{
    TArray<FSTRCollision> results;

    if (!Collisions.Contains(InSpriteName))
    {
        return results;
    }

    for (FSTRCollision collision : Collisions.Find(InSpriteName)->Collisions)
    {
        if (InType != "" && InType != collision.Type)
        {
            continue;
        }

        int32 x = InPositionX;

        if (InFacing > 0)
        {
            x += collision.X;
        }
        else
        {
            x -= collision.X;
            x -= collision.Width;
        }

        results.Add({collision.Type, x, InPositionY + collision.Y, collision.Width, collision.Height});
    }
    
    return results;
}
