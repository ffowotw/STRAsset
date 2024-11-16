#include "STRCollisionDataActions.h"
#include "STRCollisionData.h"

FSTRCollisionDataActions::FSTRCollisionDataActions(EAssetTypeCategories::Type Category)
{
    m_category = Category;
}

FText FSTRCollisionDataActions::GetName() const
{
    return FText::FromString(FString(TEXT("Collision")));
}

FColor FSTRCollisionDataActions::GetTypeColor() const
{
    return FColor::Blue;
}

UClass* FSTRCollisionDataActions::GetSupportedClass() const
{
    return USTRCollisionData::StaticClass();
}

uint32 FSTRCollisionDataActions::GetCategories()
{
    return m_category;
}
