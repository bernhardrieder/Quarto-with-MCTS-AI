// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "QuartoGameMode.h"
#include "QuartoPlayerController.h"
#include "QuartoPawn.h"

AQuartoGameMode::AQuartoGameMode()
{
	// no pawn by default
	DefaultPawnClass = AQuartoPawn::StaticClass();
	// use our own player controller class
	PlayerControllerClass = AQuartoPlayerController::StaticClass();
}
