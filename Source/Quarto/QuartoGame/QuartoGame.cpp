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
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AQuartoGame::BeginPlay()
{
	Super::BeginPlay();

	for (AQuartoToken* token : m_gameTokens)
	{
		token->m_ownerGame = this;
	}

	if(m_gameBoard)
	{
		m_gameBoard->m_ownerGame = this;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: No gameboard assigned to AQuartoGame %s"), *GetName());
	}
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
	//reset board and everything else
	m_gameBoard->Reset();
	for(AQuartoToken* token : m_gameTokens)
	{
		if(token)
		{
			token->Reset();
		}
	}
	m_gameState = EGameState::TokenSelection;
}

void AQuartoGame::HandleDrawEnd()
{
	//evaluate game
	brBool const isGameWon = m_gameBoard && m_gameBoard->GetData().GetStatus() == QuartoBoardData::GameStatus::End;
	brBool const canContinuePlaying = m_gameBoard && m_gameBoard->GetData().GetNumberOfFreeSlots() > 0;

	if(isGameWon)
	{
		//broadcast event
	}

	m_gameState = isGameWon || !canContinuePlaying ? EGameState::GameEnd : EGameState::TokenSelection;
}

void AQuartoGame::HandleGameEnd()
{
	//broadcast event
	m_gameState = EGameState::GameStart;
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
	brBool showDebug = true; // todo: move to imgui
	if (m_pickedUpToken && m_gameBoard 
		&& m_gameBoard->CanFindFreeSlot(FetchMouseCursorTargetHitResult(), showDebug))
	{
		m_gameBoard->HoverTokenOverLastFoundFreeSlot(m_pickedUpToken);
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
		m_pickedUpToken->StartHoverOver(m_pickedUpToken->GetActorLocation());
		m_gameState = EGameState::SlotSelection;
	}
}

void AQuartoGame::DiscardPickedUpToken()
{
	if(m_pickedUpToken)
	{
		m_pickedUpToken->Reset();
		m_pickedUpToken = nullptr;
		m_gameState = EGameState::TokenSelection;
	}
}

void AQuartoGame::PlaceTokenOnFocusedSlot()
{
	if (m_pickedUpToken && m_gameBoard
		&& m_gameBoard->CanFindFreeSlot(FetchMouseCursorTargetHitResult(), false))
	{
		m_gameBoard->PlaceTokenOnLastFoundFreeSlot(m_pickedUpToken);
		m_pickedUpToken = nullptr;

		m_gameState = EGameState::DrawEnd;
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


	FCollisionQueryParams queryParams = FCollisionQueryParams::DefaultQueryParam;
	if(m_gameState == EGameState::SlotSelection)
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
