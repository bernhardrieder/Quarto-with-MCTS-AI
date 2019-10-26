// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Common/UnrealCommon.h"
#include "GameFramework/Pawn.h"
#include "QuartoCommon.h"
#include "QuartoGame.generated.h"

struct QuartoBoardData;
class AQuartoBoard;
class AQuartoToken;
class UQuartoBoardSlotComponent;

UCLASS(config=Game)
class AQuartoGame : public APawn
{
	GENERATED_UCLASS_BODY()

	enum class EGameState : brU8
	{
		GameStart,
		TokenSelection,
		SlotSelection,
		NpcMoveSelection,
		DrawEnd,
		GameEnd
	};

	enum class EPlayer : brU32
	{
		Player_1 = 0,
		Player_2,
		NPC_1,
		NPC_2,
		Count
	};
	
protected:
	UPROPERTY(EditInstanceOnly, Category = "Game Components", BlueprintReadOnly, meta = (DisplayName = "Game Board"))
	AQuartoBoard* m_gameBoard;

	UPROPERTY(EditInstanceOnly, Category = "Game Components", BlueprintReadOnly, meta = (DisplayName = "Base Tokens"))
	TArray<AQuartoToken*> m_gameTokens;

public:
	void Tick(float DeltaSeconds) override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	void BeginPlay() override;

private:
	void HandleGameStart();
	void HandleDrawEnd();
	void HandleGameEnd();
	void HandleTokenSelection();
	void HandleSlotSelection();
	void HandleNpcMoveSelection();

	/** Game Specifics */

	/** Player Input */
	void HandlePlayerSelectInput();
	void PickUpFocusedToken();
	void DiscardPickedUpToken();
	void PlaceTokenOnFocusedSlot();

	/** Helper */
	FHitResult FetchMouseCursorTargetHitResult() const;
	AQuartoToken* FindToken(const FHitResult& hitResult) const;
	EPlayer GetNextPlayer(EPlayer currentPlayer);
	static constexpr brBool IsPlayerNpc(EPlayer player) { return player == EPlayer::NPC_1 || player == EPlayer::NPC_2; }
	
private:
	EGameState m_gameState;
	AQuartoToken* m_pickedUpToken;
	AQuartoToken* m_focusedToken;
	EPlayer m_players[QUARTO_NUM_OF_PLAYERS];
	EPlayer m_currentPlayer;
};