#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class FSTRMeshArrayActions : public FAssetTypeActions_Base
{
public:
    FSTRMeshArrayActions(EAssetTypeCategories::Type Category);

public:
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual uint32 GetCategories() override;

private:
    EAssetTypeCategories::Type m_category;
};