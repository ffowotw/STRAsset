#include "STRCamera.h"

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

	float positionScale = 1 / float(1000);
	float rotationScale = 1 / float(180);

	m_position = FVector(m_transform.PositionX * positionScale, m_transform.PositionY * positionScale, m_transform.PositionZ * positionScale);
	m_rotation = FRotator(m_transform.RotationX * rotationScale, m_transform.RotationY * rotationScale, m_transform.RotationZ * rotationScale);

	m_cameraComponent->SetWorldLocation(m_position);
	m_cameraComponent->SetWorldRotation(m_rotation);
}

void ASTRCamera::Ticking()
{
	float positionScale = 1 / float(1000);
	float rotationScale = 1 / float(180);

	FVector targetPosition = FVector(m_transform.PositionX * positionScale, m_transform.PositionY * positionScale, m_transform.PositionZ * positionScale);
	FRotator targetRotation = FRotator(m_transform.RotationX * rotationScale, m_transform.RotationY * rotationScale, m_transform.RotationZ * rotationScale);

	m_position = targetPosition * (6.0 / 60.0) + m_position * (1 - (6.0 / 60.0));
	m_rotation = targetRotation * (6.0 / 60.0) + m_rotation * (1 - (6.0 / 60.0));

	m_cameraComponent->SetWorldLocation(m_position);
	m_cameraComponent->SetWorldRotation(m_rotation);
}
