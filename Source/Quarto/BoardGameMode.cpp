// F


#include "BoardGameMode.h"
#include "BoardGamePlayerController.h"
#include "QuartoGame/QuartoGame.h"

ABoardGameMode::ABoardGameMode()
{
	DefaultPawnClass = AQuartoGame::StaticClass();
	PlayerControllerClass = ABoardGamePlayerController::StaticClass();
}
