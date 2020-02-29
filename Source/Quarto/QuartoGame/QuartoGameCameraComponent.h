// F

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Common/UnrealCommon.h"
#include "QuartoGameCameraComponent.generated.h"

class AActor;
class APlayerController;

/**
 * 
 */
UCLASS()
class QUARTO_API UQuartoGameCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

	// most of the parameters here are quite big because of the size of the board which should be redone at some point
protected:
	UPROPERTY(EditInstanceOnly, Category = "Settings", BlueprintReadWrite, meta = (DisplayName = "Initial Rotation"))
	FRotator m_initialRotation {45.f, 0.f, 0.f};
	
	UPROPERTY(EditInstanceOnly, Category = "Settings", BlueprintReadWrite, meta = (DisplayName = "Initial Distance"))
	float m_initialDistance = 3000.f; 

	UPROPERTY(EditInstanceOnly, Category = "Settings", BlueprintReadWrite, meta = (DisplayName = "Movement Speed"))
	float m_movementSpeed = 250.f;

	UPROPERTY(EditInstanceOnly, Category = "Settings", BlueprintReadWrite, meta = (DisplayName = "Zoom Speed"))
	float m_zoomSpeed = 2000.f;

	UPROPERTY(EditInstanceOnly, Category = "Settings", BlueprintReadWrite, meta = (DisplayName = "Min/Max Zoom"))
	FVector2D m_minMaxZoom {1500.f, 3000.f};

	UPROPERTY(EditInstanceOnly, Category = "Settings", BlueprintReadWrite, meta = (DisplayName = "Min/Max Pitch"))
	FVector2D m_minMaxPitch {10.f, 80.f};

public:
	void AttachToCamera(APlayerController* playerController);
	void DetachFromCamera(APlayerController* playerController);
	void SetCameraMovementEnabled(brBool enabled) { m_isMovementEnabled = enabled; }
	void HandleCameraInput(brFloat pitchInput, brFloat yawInput, brFloat zoomInput, brFloat deltaSeconds);

private:
	void SetCameraRotationAndLocation(FRotator const& rotation, brFloat distance);
	
private:
	APlayerController* m_attachedPlayerController = nullptr;
	AActor* m_previousViewTarget = nullptr;
	FRotator m_currentRotation;
	brFloat m_currentDistance = 0.f;
	brBool m_isMovementEnabled = false;
};
