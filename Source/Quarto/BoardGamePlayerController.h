// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BoardGamePlayerController.generated.h"

/** PlayerController class used to enable cursor */
UCLASS()
class ABoardGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABoardGamePlayerController();
};


