// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BoardGamePlayerController.h"

ABoardGamePlayerController::ABoardGamePlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}
