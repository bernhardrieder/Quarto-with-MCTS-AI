// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "QuartoGame.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "QuartoBoard.h"
#include "QuartoBoardSlotComponent.h"
#include "QuartoToken.h"

AQuartoGame::AQuartoGame(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
, m_gameBoard(nullptr)
, m_gameState(EGameState::Initialization)
, m_pickedUpToken(nullptr)
, m_focusedToken(nullptr)
, m_focusedSlot(nullptr)
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AQuartoGame::BeginPlay()
{
	Super::BeginPlay();
	m_gameState = EGameState::TokenSelection;
}

void AQuartoGame::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch(m_gameState)
	{
	case EGameState::TokenSelection: 
		HandleTokenSelection();
		break;
	case EGameState::SlotSelection: 
		HandleSlotSelection();
		break;
	default:
		break;
	}
}

void AQuartoGame::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("PickUpFocusedToken", EInputEvent::IE_Pressed, this, &AQuartoGame::PickUpFocusedToken);
	PlayerInputComponent->BindAction("DiscardPickedUpToken", EInputEvent::IE_Pressed, this, &AQuartoGame::DiscardPickedUpToken);
}

void AQuartoGame::HandleTokenSelection()
{
	AQuartoToken* token = FindToken(FetchMouseCursorTargetHitResult());

	if(m_focusedToken && m_focusedToken != token)
	{
		m_focusedToken->ShowHighlightForPlayer(false);
		m_focusedToken = nullptr;
	}

	if (token)
	{
		m_focusedToken = token;
		m_focusedToken->ShowHighlightForPlayer(true);
	}
}

void AQuartoGame::HandleSlotSelection()
{
	if (m_gameBoard && m_gameState == EGameState::SlotSelection)
	{
		bool showDebug = true;
		m_focusedSlot = m_gameBoard->FindSlot(FetchMouseCursorTargetHitResult(), showDebug);
	}
}

void AQuartoGame::PickUpFocusedToken()
{
	if(!m_pickedUpToken && m_focusedToken && m_gameState == EGameState::TokenSelection)
	{
		m_pickedUpToken = m_focusedToken;
		m_focusedToken = nullptr;
		m_pickedUpToken->SetActorLocation(m_pickedUpToken->GetActorLocation() + FVector::UpVector * 100.f);
		m_gameState = EGameState::SlotSelection;
	}
}

void AQuartoGame::DiscardPickedUpToken()
{
	if(m_pickedUpToken)
	{
		m_pickedUpToken->SetActorLocation(m_pickedUpToken->GetActorLocation() - FVector::UpVector * 100.f);
		m_pickedUpToken->ShowHighlightForPlayer(false);
		m_pickedUpToken = nullptr;
		m_gameState = EGameState::TokenSelection;
	}
}

FHitResult AQuartoGame::FetchMouseCursorTargetHitResult() const
{
	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (!playerController)
	{
		return FHitResult();
	}

	FVector start, dir, end;
	playerController->DeprojectMousePositionToWorld(start, dir);
	end = start + (dir * 10000.f);

	FHitResult hitResult;
	GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility);
	return hitResult;
}

AQuartoToken* AQuartoGame::FindToken(const FHitResult& hitResult) const
{
	if (hitResult.IsValidBlockingHit() && hitResult.Actor.IsValid())
	{
		return Cast<AQuartoToken>(hitResult.Actor.Get());
	}
	return nullptr;
}

//
//void AQuartoGame::TriggerClick()
//{
//	if (CurrentBlockFocus)
//	{
//		CurrentBlockFocus->HandleClicked();
//	}
//}
