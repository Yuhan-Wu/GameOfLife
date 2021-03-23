// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GameOfLifePlayerController.h"

AGameOfLifePlayerController::AGameOfLifePlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}
