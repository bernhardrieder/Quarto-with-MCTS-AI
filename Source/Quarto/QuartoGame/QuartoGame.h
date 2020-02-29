// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "GameFramework/Pawn.h"
#include "Quarto/Common/UnrealCommon.h"
#include "Quarto/QuartoGame/QuartoCommon.h"
#include "QuartoGame.generated.h"

struct QuartoBoardData;
class APlayerController;
class AQuartoBoard;
class AQuartoToken;
class UQuartoBoardSlotComponent;
class UQuartoGameCameraComponent;

namespace ai
{
	namespace mcts
	{
		class MonteCarloTreeSearch;
	}
}

/*	----- Quarto Gameflow ----- (http://www.ludoteka.com/quarto-en.html)
 *	Players move alternatively, placing one piece on the board; once inserted, pieces cannot be moved.
 *	One of the more special characteristics of this game is that the choice of the piece to be placed on the board is not made by the same player who places it; it is the opponent who, after doing his move, decides which will be the next piece to place.
 *	So, each turn consists of two actions:
 *		1. Place on the board the piece given by the opponent.
 *		2. Give to the opponent the piece to be placed in the next move.
 *	In the first turn of the game, the player who starts has only to choose one piece for the opponent.
 */
UENUM(BlueprintType)
enum class EQuartoGameState : uint8
{
	GameStart,
	SlotSelection_Human,
	SlotSelection_NPC,
	TokenSelection_Human,
	TokenSelection_NPC,
	GameBoardValidation,
	DrawEnd,
	GameEnd
};

UENUM(BlueprintType)
enum class EQuartoPlayer : uint8
{
	Player_1 = 0,
	Player_2,
	NPC_1,
	NPC_2,
	Count
};

UCLASS(config=Game)
class AQuartoGame : public APawn
{
	GENERATED_UCLASS_BODY()
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStateChangedEvent, EQuartoGameState, oldState, EQuartoGameState, newState);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrentPlayerChangedEvent, EQuartoPlayer, oldPlayer, EQuartoPlayer, newPlayer);
	
protected:
	UPROPERTY(EditInstanceOnly, Category = "Components", BlueprintReadOnly, meta = (DisplayName = "Game Board"))
	AQuartoBoard* m_gameBoard;

	UPROPERTY(EditInstanceOnly, Category = "Components", BlueprintReadOnly, meta = (DisplayName = "Base Tokens"))
	TArray<AQuartoToken*> m_gameTokens;

	UPROPERTY(EditInstanceOnly, Category = "Settings", BlueprintReadWrite, meta = (DisplayName = "Player 1"))
	EQuartoPlayerType m_player1;

	UPROPERTY(EditInstanceOnly, Category = "Settings", BlueprintReadWrite, meta = (DisplayName = "Player 2"))
	EQuartoPlayerType m_player2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UQuartoGameCameraComponent* m_gameCamera;

	UPROPERTY(EditInstanceOnly, Category = "AI Settings", BlueprintReadWrite, meta = (DisplayName = "Max time to think about next move in seconds"))
	float m_maxAiThinkTimeForNextMove;

	UPROPERTY(EditInstanceOnly, Category = "AI Settings", BlueprintReadWrite, meta = (DisplayName = "Max time to think about next opponent token in seconds"))
	float m_maxAiThinkTimeForNextOpponentToken;

public:
	UPROPERTY(BlueprintAssignable)
	FOnGameStateChangedEvent OnGameStateChangedEvent;

	UPROPERTY(BlueprintAssignable)
	FOnCurrentPlayerChangedEvent OnCurrentPlayerChangedEvent;

public:
	UFUNCTION(BlueprintCallable)
	void StartPlaying(APlayerController* playerController);

	UFUNCTION(BlueprintCallable)
	void StopPlaying(APlayerController* playerController);

	void Tick(float DeltaSeconds) override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Game State Handlers*/
	void HandleGameStart();
	void HandleDrawEnd();
	void HandleGameEnd();
	void HandleTokenSelection_Human();
	void HandleTokenSelection_NPC();
	void HandleSlotSelection_Human();
	void HandleSlotSelection_NPC();
	void HandleGameBoardValidation();

	/** Player Input */
	void HandlePlayerSelectInput();
	void SetCameraMovementEnabled();
	void SetCameraMovementDisabled();
	void HandleCameraMovementInputAxisX(brFloat input);
	void HandleCameraMovementInputAxisY(brFloat input);
	void HandleCameraZoomInput(brFloat);
	void PickUpFocusedToken();
	void PickUpToken(AQuartoToken* token);
	void PlaceTokenOnFocusedSlot();

	/** Setters */
	void SetCurrentPlayer(EQuartoPlayer player);
	void SetGameState(EQuartoGameState gameState);

	/** Helper */
	FHitResult FetchMouseCursorTargetHitResult() const;
	AQuartoToken* FindToken(const FHitResult& hitResult) const;
	EQuartoPlayer GetNextPlayer(EQuartoPlayer currentPlayer);
	static constexpr brBool IsPlayerNpc(EQuartoPlayer player) { return player == EQuartoPlayer::NPC_1 || player == EQuartoPlayer::NPC_2; }
	static FString GetPlayerName(EQuartoPlayer player);
	
private:
	EQuartoGameState m_gameState;
	AQuartoToken* m_pickedUpToken;
	AQuartoToken* m_focusedToken;
	EQuartoPlayer m_players[QUARTO_NUM_OF_PLAYERS];
	EQuartoPlayer m_currentPlayer;
	ai::mcts::MonteCarloTreeSearch* m_mctsAi;

	brBool m_isPlayed = false;
	
#ifdef DEBUG_BUILD
	EQuartoGameState m_oldGameState;
#endif
};
