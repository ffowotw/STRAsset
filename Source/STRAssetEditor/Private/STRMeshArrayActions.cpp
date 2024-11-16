#include "STRMeshArrayActions.h"
#include "STRMeshArray.h"

FSTRMeshArrayActions::FSTRMeshArrayActions(EAssetTypeCategories::Type Category)
{
    m_category = Category;
}

FText FSTRMeshArrayActions::GetName() const
{
    return FText::FromString(FString(TEXT("MeshArray")));
}

FColor FSTRMeshArrayActions::GetTypeColor() const
{
    return FColor::Blue;
}

UClass* FSTRMeshArrayActions::GetSupportedClass() const
{
    return USTRMeshArray::StaticClass();
}

uint32 FSTRMeshArrayActions::GetCategories()
{
    return m_category;
}
