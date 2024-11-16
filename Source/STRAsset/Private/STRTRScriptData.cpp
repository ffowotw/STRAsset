#include "STRTRScriptData.h"

void USTRTRScriptData::SetScriptText(FString InString)
{
    m_scriptText = InString;
}

void USTRTRScriptData::GenerateSubroutines()
{
    Subroutines.Empty();

    TArray<FString> scriptLines;
    m_scriptText.ParseIntoArray(scriptLines, TEXT("\n"), true);

    FString currentSubroutine = "";

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

        if (currentSubroutine == "")
        {
            currentSubroutine = value;

            int32 startIndex = currentSubroutine.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, 0);
            int32 endIndex = currentSubroutine.Find("\"", ESearchCase::IgnoreCase, ESearchDir::Type::FromStart, startIndex + 1);
            
            currentSubroutine = currentSubroutine.Mid(startIndex + 1, endIndex - startIndex - 1);
        
            FSTRSubroutine subroutine = { header == "beginState" ? STRSubroutineType::STATE : STRSubroutineType::FUNCTION };

            Subroutines.Add(currentSubroutine, subroutine);
        }
        else if (header == "endState" || header == "endFunction")
        {
            currentSubroutine = "";

            continue;
        }
        else
        {
            Subroutines[currentSubroutine].SubroutineValues.Add({
                header,
                value
            });
        }
    }
}
