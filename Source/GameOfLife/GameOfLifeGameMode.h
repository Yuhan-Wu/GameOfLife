// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameOfLifeGameMode.generated.h"

/** GameMode class to specify pawn and playercontroller */
UCLASS(minimalapi)
class AGameOfLifeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGameOfLifeGameMode();
};



