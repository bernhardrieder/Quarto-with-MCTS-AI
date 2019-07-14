// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "QuartoGame.generated.h"

class AQuartoBoard;
class AQuartoToken;
class UQuartoBoardSlotComponent;

UCLASS(config=Game)
class AQuartoGame : public APawn
{
	GENERATED_UCLASS_BODY()

	enum class EGameState : uint8
	{
		Initialization,
		TokenSelection,
		SlotSelection,
		GameEnd
	};

protected:
	UPROPERTY(EditInstanceOnly, Category = "Game Components", BlueprintReadOnly, meta = (DisplayName = "Game Board"))
	AQuartoBoard* m_gameBoard;

	UPROPERTY(EditInstanceOnly, Category = "Game Components", BlueprintReadOnly, meta = (DisplayName = "Base Tokens"))
	TArray<AQuartoToken*> m_baseTokens;

public:
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void HandleTokenSelection();
	void HandleSlotSelection();

	void PickUpFocusedToken();
	void DiscardPickedUpToken();

	FHitResult FetchMouseCursorTargetHitResult() const;
	AQuartoToken* FindToken(const FHitResult& hitResult) const;

private:
	EGameState m_gameState;
	AQuartoToken* m_pickedUpToken;
	AQuartoToken* m_focusedToken;
	UQuartoBoardSlotComponent* m_focusedSlot;

};
