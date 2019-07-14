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
, m_gameState(EGameState::GameStart)
, m_pickedUpToken(nullptr)
, m_focusedToken(nullptr)
, m_focusedSlot(nullptr)
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AQuartoGame::BeginPlay()
{
	Super::BeginPlay();
}

void AQuartoGame::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch(m_gameState)
	{
	case EGameState::GameStart:
		HandleGameStart();
		break;
	case EGameState::TokenSelection: 
		HandleTokenSelection();
		break;
	case EGameState::SlotSelection: 
		HandleSlotSelection();
		break;
	case EGameState::DrawEnd: 
		HandleDrawEnd();
		break;
	case EGameState::GameEnd: 
		HandleGameEnd();
		break;
	default:
		break;
	}
}

void AQuartoGame::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("QuartoGame_PlayerSelect", EInputEvent::IE_Pressed, this, &AQuartoGame::HandlePlayerSelectInput);
	PlayerInputComponent->BindAction("QuartoGame_PlayerAbort", EInputEvent::IE_Pressed, this, &AQuartoGame::DiscardPickedUpToken);
}

void AQuartoGame::HandleGameStart()
{
	//reset board and everything
	m_gameState = EGameState::TokenSelection;
}

void AQuartoGame::HandleDrawEnd()
{
	//evaluate game
	bool gameEnd = false;
	m_gameState = gameEnd ? EGameState::GameEnd : EGameState::GameStart;
}

void AQuartoGame::HandleGameEnd()
{
}

void AQuartoGame::HandleTokenSelection()
{
	AQuartoToken* token = FindToken(FetchMouseCursorTargetHitResult());

	if(m_focusedToken && m_focusedToken != token)
	{
		m_focusedToken->ShowHighlightForPlayer(false);
		m_focusedToken = nullptr;
	}

	if (token && !token->IsPlacedOnBoard())
	{
		m_focusedToken = token;
		m_focusedToken->ShowHighlightForPlayer(true);
	}
}

void AQuartoGame::HandleSlotSelection()
{
	if (m_pickedUpToken && m_gameBoard)
	{
		bool showDebug = true; // move to imgui
		auto slot = m_gameBoard->FindSlot(FetchMouseCursorTargetHitResult(), showDebug);

		if(slot && !slot->HasPlacedToken())
		{
			m_focusedSlot = slot;
			m_focusedSlot->HoverToken(m_pickedUpToken);
		}
	}
}

void AQuartoGame::HandlePlayerSelectInput()
{
	switch(m_gameState)
	{
	case EGameState::TokenSelection: 
		PickUpFocusedToken();
		break;
	case EGameState::SlotSelection:
		PlaceTokenOnFocusedSlot();
		break;
	default:
		break;
	}
}

void AQuartoGame::PickUpFocusedToken()
{
	if(!m_pickedUpToken 
		&& m_focusedToken 
		&& !m_focusedToken->IsPlacedOnBoard())
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
		m_pickedUpToken->RemoveFromBoard();
		m_pickedUpToken = nullptr;
		m_gameState = EGameState::TokenSelection;
	}
}

void AQuartoGame::PlaceTokenOnFocusedSlot()
{
	if(!m_focusedSlot 
		|| !m_pickedUpToken 
		|| m_focusedSlot->HasPlacedToken())
	{
		return;
	}

	m_pickedUpToken->PlaceOnBoard(m_focusedSlot);
	m_pickedUpToken = nullptr;

	m_gameState = EGameState::DrawEnd;

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


	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	if(m_pickedUpToken)
	{
		queryParams.AddIgnoredActor(m_pickedUpToken);
	}

	FHitResult hitResult;
	GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, queryParams);
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
