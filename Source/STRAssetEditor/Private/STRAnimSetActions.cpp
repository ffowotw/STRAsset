#include "STRAnimSetActions.h"
#include "Anim/STRAnimSet.h"

FSTRAnimSetActions::FSTRAnimSetActions(EAssetTypeCategories::Type Category)
{
    m_category = Category;
}

FText FSTRAnimSetActions::GetName() const
{
    return FText::FromString(FString(TEXT("AnimSet")));
}

FColor FSTRAnimSetActions::GetTypeColor() const
{
    return FColor::Blue;
}

UClass* FSTRAnimSetActions::GetSupportedClass() const
{
    return USTRAnimSet::StaticClass();
}

uint32 FSTRAnimSetActions::GetCategories()
{
    return m_category;
}
