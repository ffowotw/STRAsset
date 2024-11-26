#include "Camera/STRCamera.h"

ASTRCamera::ASTRCamera()
{
	m_cameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CAM"));
	m_cameraComponent->FieldOfView = 54;
	m_cameraComponent->bConstrainAspectRatio = true;
	m_cameraComponent->AspectRatio = float(16) / float(9);
	// m_cameraComponent->SetRelativeLocation(FVector::ZeroVector);

	m_transform = {
		0,
		// 540000,
		// 106423,
		1250000,
		250000,
		450,
		-16200,
		0
	};

	m_position = m_transform.GetPosition();
	m_rotation = m_transform.GetRotation();

	m_cameraComponent->SetWorldLocation(m_position);
	m_cameraComponent->SetWorldRotation(m_rotation);
}

void ASTRCamera::Ticking()
{
	FVector targetPosition = m_transform.GetPosition();
	FRotator targetRotation = m_transform.GetRotation();

	m_position = targetPosition * 0.15 + m_position * (1 - 0.15);
	m_rotation = targetRotation * 0.15 + m_rotation * (1 - 0.15);

	m_cameraComponent->SetWorldLocation(m_position);
	m_cameraComponent->SetWorldRotation(m_rotation);
}
