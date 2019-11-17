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
#include "AI/MonteCarloTreeSearch.h"

AQuartoGame::AQuartoGame(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, m_gameBoard(nullptr)
	, m_gameState(EGameState::GameStart)
	, m_pickedUpToken(nullptr)
	, m_focusedToken(nullptr)
	, m_currentPlayer(EPlayer::Player_1)
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AQuartoGame::BeginPlay()
{
	Super::BeginPlay();

	m_mctsAi = new ai::mcts::MonteCarloTreeSearch(5.f);
	
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

void AQuartoGame::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete m_mctsAi;
	Super::EndPlay(EndPlayReason);
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
	case EGameState::NpcMoveSelection:
		HandleNpcMoveSelection();
		break;
	case EGameState::DrawEnd: 
		HandleDrawEnd();
		break;
	case EGameState::GameEnd: 
	default:
		HandleGameEnd();
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
	m_players[0] = EPlayer::Player_1;
	m_players[1] = EPlayer::NPC_1;
	m_currentPlayer = m_players[0];
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

	m_currentPlayer = GetNextPlayer(m_currentPlayer);

	if(isGameWon || !canContinuePlaying)
	{
		m_gameState = EGameState::GameEnd;
	}
	else
	{
		m_gameState = IsPlayerNpc(m_currentPlayer) ? EGameState::NpcMoveSelection : EGameState::TokenSelection;
	}
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

void AQuartoGame::HandleNpcMoveSelection()
{
	if(m_gameBoard)
	{
		if(!m_mctsAi->IsLookingForNextMove() && !m_mctsAi->HasFoundNextMove())
		{
			m_mctsAi->FindNextMove(
				m_gameBoard->GetData(),
				static_cast<brU32>(m_currentPlayer),
				static_cast<brU32>(GetNextPlayer(m_currentPlayer))
			);
		}
		
		if(m_mctsAi->IsLookingForNextMove() && !m_mctsAi->HasFoundNextMove())
		{
			// do random stuff
			return;
		}
		
		std::tuple<QuartoTokenData, QuartoBoardSlotCoordinates> move = m_mctsAi->GetNextMove();
		QuartoTokenData& moveToken = std::get<0>(move);
		QuartoBoardSlotCoordinates moveCoordinates = std::get<1>(move);

		AQuartoToken** token = m_gameTokens.FindByPredicate([&moveToken](AQuartoToken* t) { return t && t->GetData() == moveToken; });
		if(token && *token)
		{
			m_gameBoard->HoverTokenOverSlot(*token, moveCoordinates);
			//wait
			m_gameBoard->PlaceTokenOnBoardSlot(*token, moveCoordinates);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: NPC couldn't find token to play!"));
		}
	}
	
	m_gameState = EGameState::DrawEnd;
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

AQuartoGame::EPlayer AQuartoGame::GetNextPlayer(EPlayer currentPlayer)
{
	//in case I ever decide to make a custom Quarto with more than 2 players
	auto const itBegin = std::begin(m_players);
	auto const itEnd = std::end(m_players);
	auto const it = std::find(itBegin, itEnd, currentPlayer);
	if(it != itEnd)
	{
		brU32 index = it - itBegin;
		currentPlayer = m_players[++index % QUARTO_NUM_OF_PLAYERS];
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Couldn't find next player! Did you specify the players correctly?"));
	}

	return currentPlayer;
}
