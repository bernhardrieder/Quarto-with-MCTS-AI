// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Common/UnrealCommon.h"
#include "GameFramework/Pawn.h"
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
		Human = 0,
		NPC,
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
	static EPlayer GetNextPlayer(EPlayer currentPlayer);
	
private:
	EGameState m_gameState;
	AQuartoToken* m_pickedUpToken;
	AQuartoToken* m_focusedToken;
	EPlayer m_currentPlayer;
};