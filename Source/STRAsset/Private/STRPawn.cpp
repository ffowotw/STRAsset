#include "STRPawn.h"
#include "STRGameMode.h"

void ASTRPawn::BeginPlay()
{
	Super::BeginPlay();
	
	m_gameMode = Cast<ASTRGameMode>(GetWorld()->GetAuthGameMode());

	m_playerIndex = m_gameMode->AssignPlayer(this);

    PrimaryActorTick.bStartWithTickEnabled = false;
    PrimaryActorTick.bCanEverTick = false;
}

void ASTRPawn::SetPlayer(int32 InIndex)
{
	m_playerIndex = InIndex;
}

void ASTRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindKey(EKeys::AnyKey, IE_Pressed, this, &ASTRPawn::OnKeyPressed);
	PlayerInputComponent->BindKey(EKeys::AnyKey, IE_Released, this, &ASTRPawn::OnKeyReleased);
}

void ASTRPawn::OnKeyPressed(FKey InKey)
{
	if (m_gameMode == nullptr)
	{
		return;
	}

	m_gameMode->PlayerInput(m_playerIndex, m_buttonMappings, InKey, true);
}

void ASTRPawn::OnKeyReleased(FKey InKey)
{
	if (m_gameMode == nullptr)
	{
		return;
	}

	m_gameMode->PlayerInput(m_playerIndex, m_buttonMappings, InKey, false);
}
