// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GameOfLifeGameMode.h"
#include "GameOfLifePlayerController.h"
#include "GameOfLifePawn.h"

AGameOfLifeGameMode::AGameOfLifeGameMode()
{
	// no pawn by default
	DefaultPawnClass = AGameOfLifePawn::StaticClass();
	// use our own player controller class
	PlayerControllerClass = AGameOfLifePlayerController::StaticClass();
}
