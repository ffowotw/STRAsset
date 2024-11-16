#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "STRCharaSet.h"
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
	int32 m_hitStopFrame = 0;

	virtual void Tick(float DeltaTime) override;
	void Ticking();
	void CameraTicking();

	int32 GetHitStopFrame(int32 InAttackLevel, bool InCounterHit)
	{
		switch (InAttackLevel)
		{
		case 0:
			return 11 + (InCounterHit ? 0 : 0);
		case 1:
			return 12 + (InCounterHit ? 2 : 0);
		case 2:
			return 13 + (InCounterHit ? 4 : 0);
		case 3:
			return 14 + (InCounterHit ? 8 : 0);
		case 4:
			return 15 + (InCounterHit ? 12 : 0);
		}

		return -1;
	}
	void ApplyHitStop(int32 InHitStopFrame);

public: // Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DebugP1 = "";
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DebugP2 = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FSTRCharaSet> CharaSets;

public: // Player
	UPROPERTY()
	TArray<class ASTRPawn*> m_playerList;

	int32 AssignPlayer(class ASTRPawn* playerPawn);

    void PlayerInput(int32 InIndex, TArray<FKey> InKeyMappings, FKey InKey, bool InPressed);


public: // Chara
	UPROPERTY()
	TArray<class USTRChara*> m_charaList;
	UPROPERTY()
	TMap<int32, int32> m_playerControllingChara;

	class USTRChara* SpawnChara(FString InCharaName, FString InCharaLayer, int32& OutCharaIndex);
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
};
