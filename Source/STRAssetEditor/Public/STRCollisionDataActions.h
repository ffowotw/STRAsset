#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class FSTRCollisionDataActions : public FAssetTypeActions_Base
{
public:
    FSTRCollisionDataActions(EAssetTypeCategories::Type Category);

public:
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual uint32 GetCategories() override;

private:
    EAssetTypeCategories::Type m_category;
};
