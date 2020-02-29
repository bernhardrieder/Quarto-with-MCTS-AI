// F


#include "QuartoGameCameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void UQuartoGameCameraComponent::AttachToCamera(APlayerController* playerController)
{
	if(playerController && !m_attachedPlayerController)
	{
		m_attachedPlayerController = playerController;
		m_previousViewTarget = playerController->GetViewTarget();
		SetCameraRotationAndLocation(m_initialRotation, m_initialDistance);
		playerController->SetViewTarget(GetOwner());
	}
}

void UQuartoGameCameraComponent::DetachFromCamera(APlayerController* playerController)
{
	if(m_attachedPlayerController && m_attachedPlayerController == playerController)
	{
		m_attachedPlayerController->SetViewTarget(m_previousViewTarget);
		m_previousViewTarget = nullptr;
		m_attachedPlayerController = nullptr;
	}
}

void UQuartoGameCameraComponent::HandleCameraInput(brFloat pitchInput, brFloat yawInput, brFloat zoomInput, brFloat deltaSeconds)
{
	if(m_isMovementEnabled)
	{
		FRotator const newRotation = m_currentRotation + (FRotator(pitchInput, yawInput, 0.f) * m_movementSpeed * deltaSeconds);
		brFloat const newDistance = m_currentDistance + (zoomInput * m_zoomSpeed * deltaSeconds);

		SetCameraRotationAndLocation(newRotation, newDistance);
	}
}

void UQuartoGameCameraComponent::SetCameraRotationAndLocation(FRotator const& rotation, brFloat distance)
{
	m_currentDistance = FMath::Clamp(distance, m_minMaxZoom.X, m_minMaxZoom.Y);
	m_currentRotation = rotation;
	m_currentRotation.Pitch = FMath::ClampAngle(m_currentRotation.Pitch, m_minMaxPitch.X, m_minMaxPitch.Y);
	
	FVector const newLocation = m_currentRotation.Quaternion().GetForwardVector() * m_currentDistance;
	FRotator const newRotation = UKismetMathLibrary::FindLookAtRotation(newLocation, GetOwner()->GetActorLocation());
	
	SetWorldLocationAndRotation(newLocation, newRotation);
}
