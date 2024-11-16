#include "STRAnimArrayActions.h"
#include "Anim/STRAnimArray.h"

FSTRAnimArrayActions::FSTRAnimArrayActions(EAssetTypeCategories::Type Category)
{
    m_category = Category;
}

FText FSTRAnimArrayActions::GetName() const
{
    return FText::FromString(FString(TEXT("AnimArray")));
}

FColor FSTRAnimArrayActions::GetTypeColor() const
{
    return FColor::Blue;
}

UClass* FSTRAnimArrayActions::GetSupportedClass() const
{
    return USTRAnimArray::StaticClass();
}

uint32 FSTRAnimArrayActions::GetCategories()
{
    return m_category;
}
