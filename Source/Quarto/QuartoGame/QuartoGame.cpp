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
	, m_player1(EQuartoPlayer::Human)
	, m_player2(EQuartoPlayer::Human)
	, m_maxAiThinkTimeForNextMove(5.0f)
	, m_maxAiThinkTimeForNextOpponentToken(0.5f)
	, m_gameState(EQuartoGameState::GameStart)
	, m_oldGameState(EQuartoGameState::GameEnd)
	, m_pickedUpToken(nullptr)
	, m_focusedToken(nullptr)
	, m_currentPlayer(EPlayer::Player_1)
	, m_mctsAi(nullptr)
{
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AQuartoGame::BeginPlay()
{
	Super::BeginPlay();

	m_mctsAi = new ai::mcts::MonteCarloTreeSearch(m_maxAiThinkTimeForNextMove, m_maxAiThinkTimeForNextOpponentToken);
	
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

	if(m_oldGameState != m_gameState)
	{
		UE_LOG(LogTemp, Display, TEXT("GameLoop: %s - %s"), *GETENUMSTRING("EQuartoGameState", m_gameState), *GetPlayerName(m_currentPlayer));
		m_oldGameState = m_gameState;
	}
	
	switch(m_gameState)
	{
	case EQuartoGameState::GameStart:
		HandleGameStart();
		break;
	case EQuartoGameState::SlotSelection_Human: 
		HandleSlotSelection_Human();
		break;
	case EQuartoGameState::SlotSelection_NPC:
		HandleSlotSelection_NPC();
		break;
	case EQuartoGameState::TokenSelection_Human:
		HandleTokenSelection_Human();
		break;
	case EQuartoGameState::TokenSelection_NPC:
		HandleTokenSelection_NPC();
		break;
	case EQuartoGameState::DrawEnd: 
		HandleDrawEnd();
		break;
	case EQuartoGameState::GameBoardValidation:
		HandleGameBoardValidation();
		break;
	case EQuartoGameState::GameEnd:
	default:
		HandleGameEnd();
		break;
	}
}

void AQuartoGame::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("QuartoGame_PlayerSelect", EInputEvent::IE_Pressed, this, &AQuartoGame::HandlePlayerSelectInput);
	//PlayerInputComponent->BindAction("QuartoGame_PlayerAbort", EInputEvent::IE_Pressed, this, &AQuartoGame::DiscardPickedUpToken);
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

	m_players[0] = m_player1 == EQuartoPlayer::Human ? EPlayer::Player_1 : EPlayer::NPC_1;
	m_players[1] = m_player2 == EQuartoPlayer::Human ? EPlayer::Player_2 : EPlayer::NPC_2;
	m_currentPlayer = m_players[static_cast<int>(FMath::RandBool())];

	m_gameState = IsPlayerNpc(m_currentPlayer) ? EQuartoGameState::TokenSelection_NPC : EQuartoGameState::TokenSelection_Human;
}

void AQuartoGame::HandleDrawEnd()
{
	m_currentPlayer = GetNextPlayer(m_currentPlayer);
	m_gameState = IsPlayerNpc(m_currentPlayer) ? EQuartoGameState::SlotSelection_NPC : EQuartoGameState::SlotSelection_Human;
}

void AQuartoGame::HandleGameEnd()
{
	//broadcast event
	m_gameState = EQuartoGameState::GameStart;
}

void AQuartoGame::HandleTokenSelection_Human()
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

void AQuartoGame::HandleTokenSelection_NPC()
{
	if (m_gameBoard)
	{
		if (!m_mctsAi->IsLookingForNextOpponentToken() && !m_mctsAi->HasFoundNextOpponentToken())
		{
			m_mctsAi->FindNextOpponentToken(
				m_gameBoard->GetData(),
				static_cast<brU32>(GetNextPlayer(m_currentPlayer)),
				static_cast<brU32>(m_currentPlayer)
			);
		}

		if (m_mctsAi->IsLookingForNextOpponentToken() && !m_mctsAi->HasFoundNextOpponentToken())
		{
			// do random stuff
			return;
		}
		
		auto const findToken = [&](QuartoTokenData const& data) { return m_gameTokens.FindByPredicate([&data](AQuartoToken* t) { return t && t->GetData() == data; }); };
		PickUpToken(*findToken(m_mctsAi->GetNextOpponentToken()));
	}
	m_gameState = EQuartoGameState::DrawEnd;
}

void AQuartoGame::HandleSlotSelection_Human()
{
	brBool showDebug = true; // todo: move to imgui
	if (m_pickedUpToken && m_gameBoard 
		&& m_gameBoard->CanFindFreeSlot(FetchMouseCursorTargetHitResult(), showDebug))
	{
		m_gameBoard->HoverTokenOverLastFoundFreeSlot(m_pickedUpToken);
	}
}

void AQuartoGame::HandleSlotSelection_NPC()
{
	if(m_gameBoard && m_pickedUpToken)
	{
		if(!m_mctsAi->IsLookingForNextMove() && !m_mctsAi->HasFoundNextMove())
		{
			m_mctsAi->FindNextMove(
				m_pickedUpToken->GetData(),
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
		
		QuartoBoardSlotCoordinates const moveCoordinates = m_mctsAi->GetNextMoveCoordinates();

		m_gameBoard->HoverTokenOverSlot(m_pickedUpToken, moveCoordinates);
		//wait
		m_gameBoard->PlaceTokenOnBoardSlot(m_pickedUpToken, moveCoordinates);
	}
	
	m_gameState = EQuartoGameState::GameBoardValidation;
}

void AQuartoGame::HandleGameBoardValidation()
{
	//evaluate game
	brBool const isGameWon = m_gameBoard && m_gameBoard->GetData().GetStatus() == QuartoBoardData::GameStatus::End;
	brBool const canContinuePlaying = m_gameBoard && m_gameBoard->GetData().GetNumberOfFreeSlots() > 0;

	if (isGameWon)
	{
		//broadcast event
	}

	if (isGameWon || !canContinuePlaying)
	{
		m_gameState = EQuartoGameState::GameEnd;
		UE_LOG(LogTemp, Display, TEXT("%s won!"), *GetPlayerName(m_currentPlayer));
	}
	else
	{
		m_gameState = IsPlayerNpc(m_currentPlayer) ? EQuartoGameState::TokenSelection_NPC : EQuartoGameState::TokenSelection_Human;
	}

}

void AQuartoGame::HandlePlayerSelectInput()
{
	switch(m_gameState)
	{
	case EQuartoGameState::TokenSelection_Human: 
		PickUpFocusedToken();
		break;
	case EQuartoGameState::SlotSelection_Human:
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
		m_focusedToken->ShowHighlightForPlayer(false);
		PickUpToken(m_focusedToken);
		m_gameState = EQuartoGameState::DrawEnd;
	}
}

void AQuartoGame::PickUpToken(AQuartoToken* token)
{
	if(token)
	{
		m_pickedUpToken = token;
		m_focusedToken = nullptr;
		token->StartHoverOver(token->GetActorLocation());
	}
}

//void AQuartoGame::DiscardPickedUpToken()
//{
//	if(m_pickedUpToken)
//	{
//		m_pickedUpToken->Reset();
//		m_pickedUpToken = nullptr;
//		m_gameState = EQuartoGameState::TokenSelection_Human;
//	}
//}

void AQuartoGame::PlaceTokenOnFocusedSlot()
{
	if (m_pickedUpToken && m_gameBoard
		&& m_gameBoard->CanFindFreeSlot(FetchMouseCursorTargetHitResult(), false))
	{
		m_gameBoard->PlaceTokenOnLastFoundFreeSlot(m_pickedUpToken);
		m_pickedUpToken = nullptr;

		m_gameState = EQuartoGameState::GameBoardValidation;
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
	if(m_gameState == EQuartoGameState::SlotSelection_Human)
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

FString AQuartoGame::GetPlayerName(EPlayer player)
{
	switch(player)
	{
		case EPlayer::Player_1: return FString("Player_1");
		case EPlayer::Player_2: return FString("Player_2");
		case EPlayer::NPC_1: return FString("NPC_1");
		case EPlayer::NPC_2: return FString("NPC_2");
		case EPlayer::Count: 
		default:
			return FString("Invalid");
	}
}
