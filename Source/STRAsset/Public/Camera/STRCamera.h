#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "STRCamera.generated.h"

USTRUCT()
struct FTransformStruct
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	int32 PositionX;
	UPROPERTY()
	int32 PositionY;
	UPROPERTY()
	int32 PositionZ;

	UPROPERTY()
	int32 RotationX;
	UPROPERTY()
	int32 RotationY;
	UPROPERTY()
	int32 RotationZ;
};

UCLASS()
class STRASSET_API ASTRCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	ASTRCamera();

public:
	void Ticking();
	void SetCameraTransform(FTransformStruct InTransform)
	{
		m_transform = InTransform;
	}

private:
	UPROPERTY()
	FTransformStruct m_transform;

	UPROPERTY()
	TArray<FTransformStruct> m_charatransformList;

private:
	UPROPERTY()
	UCameraComponent* m_cameraComponent;

	FVector m_position;
	FRotator m_rotation;
};
