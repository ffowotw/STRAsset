#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Structures/STRDataSet.h"
#include "STRGameMode.generated.h"

UCLASS(BlueprintType)
class STRASSET_API ASTRGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public: // Game
	UPROPERTY()
	TArray<FString> m_selectingChara;

	virtual void Tick(float DeltaTime) override;
	void ObjectTicking();
	void CameraTicking();

public: // Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DebugP1 = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DebugP2 = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USTRDataArray* DataArray;

public: // Player
	UPROPERTY()
	TArray<class ASTRPawn*> m_playerList;

	int32 AssignPlayer(class ASTRPawn* playerPawn);
    void PlayerInput(int32 InIndex, TArray<FKey> InKeyMappings, FKey InKey, bool InPressed);

	TArray<FString> GetPlayerButtons()
	{
		return {
			"A",
			"B",
			"C",
			"D",
			"TAUNT"
		};
	}

public: // Object
	UPROPERTY()
	TArray<class USTRObject*> m_objectList;
	UPROPERTY()
	TArray<class USTRChara*> m_charaList;

	TArray<USTRObject*> GetObjectList()
	{
		return m_objectList;
	};

	TArray<USTRObject*> GetTickableObjectList()
	{
		TArray<USTRObject*> result;

		for (USTRObject* obj : m_objectList)
		{
			if (obj->Tickable())
			{
				result.Add(obj);
			}
		}

		return result;
	}

	UPROPERTY()
	TMap<int32, int32> m_playerControllingChara;

	void CreateObject(USTRObject* InParent, FString InObjectName, FSTRDataSet InDataSet);
	class USTRChara* CreateChara(FString InCharaName, FString InCharaLayer, int32& OutCharaIndex);
	class ASTRParticle* CreateParticle(class USTRParticleDataAsset* InParticleDataAsset, FString InParticleName);
	
	TArray<USTRChara*> GetOpponentCharaList(FString InLayer, bool InContainsIgnore = false);
	TArray<USTRChara*> GetCharaList()
	{
		return m_charaList;
	};

public: // Collision
    void DrawCollision(FSTRCollision InCollision, FColor InColor);

private: // Camera
	UPROPERTY()
	class ASTRCamera* m_camera;

	int32 m_cameraLastY = 0;
	int32 m_cameraZoomInDelayTimer = 0;
	bool m_cameraZoomInEnabled = false;
};
