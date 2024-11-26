#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Structures/STRTransform.h"
#include "STRCamera.generated.h"

UCLASS(BlueprintType)
class ASTRCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	ASTRCamera();

public:
	void Ticking();
	void SetCameraTransform(FSTRTransform InTransform)
	{
		m_transform = InTransform;
	}

private:
	UPROPERTY()
	FSTRTransform m_transform;

private:
	UPROPERTY()
	UCameraComponent* m_cameraComponent;

	FVector m_position;
	FRotator m_rotation;
};
