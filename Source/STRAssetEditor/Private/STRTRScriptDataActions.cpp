#include "STRTRScriptDataActions.h"
#include "STRTRScriptData.h"

FSTRTRScriptDataActions::FSTRTRScriptDataActions(EAssetTypeCategories::Type Category)
{
    m_category = Category;
}

FText FSTRTRScriptDataActions::GetName() const
{
    return FText::FromString(FString(TEXT("TRS")));
}

FColor FSTRTRScriptDataActions::GetTypeColor() const
{
    return FColor::Blue;
}

UClass* FSTRTRScriptDataActions::GetSupportedClass() const
{
    return USTRTRScriptData::StaticClass();
}

uint32 FSTRTRScriptDataActions::GetCategories()
{
    return m_category;
}
