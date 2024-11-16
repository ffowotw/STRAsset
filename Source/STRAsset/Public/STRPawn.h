#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STRPawn.generated.h"

UCLASS()
class STRASSET_API ASTRPawn : public APawn
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:	
	virtual void SetPlayer(int32 InIndex);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	ASTRGameMode* m_gameMode;
	
	UPROPERTY()
	int32 m_playerIndex;

	UPROPERTY()
	TArray<FKey> m_keyMappings = TArray<FKey> {
	};

	UPROPERTY()
	TArray<FKey> m_buttonMappings = TArray<FKey> {
		EKeys::Gamepad_DPad_Up, 			// Up
		EKeys::Gamepad_DPad_Down, 			// Down
		EKeys::Gamepad_DPad_Right, 			// Right
		EKeys::Gamepad_DPad_Left, 			// Left
		EKeys::Gamepad_FaceButton_Left, 	// A
		EKeys::Gamepad_FaceButton_Top, 		// B
		EKeys::Gamepad_FaceButton_Right, 	// C
		EKeys::Gamepad_FaceButton_Bottom 	// D
		// EKeys::W, // Up
		// EKeys::S, // Down
		// EKeys::D, // Right
		// EKeys::A, // Left
		// EKeys::U, // A
		// EKeys::I, // B
		// EKeys::K, // C
		// EKeys::J, // D
	};

private:
	void OnKeyPressed(FKey InKey);
	void OnKeyReleased(FKey InKey);
};
