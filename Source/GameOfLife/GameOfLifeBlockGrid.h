// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "GameOfLifeBlockGrid.generated.h"

/** Class used to spawn blocks and manage score */
UCLASS(minimalapi)
class AGameOfLifeBlockGrid : public AActor
{
	GENERATED_BODY()

	/** Dummy root component */
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* DummyRoot;

	// Speed
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UTextRenderComponent* SpeedText;

	// Pause
	UPROPERTY(Category = Grid, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UTextRenderComponent* PauseText;

public:
	AGameOfLifeBlockGrid();

	/** Number of blocks along each side of grid */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	int32 Size;

	/** Spacing of blocks */
	UPROPERTY(Category=Grid, EditAnywhere, BlueprintReadOnly)
	float BlockSpacing;

	// Gameplay info
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadOnly)
	bool IsRunning;
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadOnly)
	float RestTime;

	void RegisterHighlightBlock(FVector2D Coordinate); // For clicking
	void RegisterHighlightBlock(FVector2D Coordinate, TArray<FVector2D>& TempList); // For iteration

protected:
	// Begin AActor interface
	virtual void BeginPlay() override;
	// End AActor interface

	void ToggleStart();
	void TriggerGridUpdate();
	void RegisterNeighbor(FVector2D Coordinate, TArray<FVector2D>& TempList);

	void SetUpTimer();
	void ClearTimer();

	void SpeedUp();
	void SlowDown();
	void UpdateSpeedText();

	void Restart();

	TArray<float> SpeedOptions = {0.25f, 0.5f, 1.f, 1.5f, 2.f};
	int CurSpeedOption;

	// Grid info
	TArray<class AGameOfLifeBlock*> GridInfo;
	TArray<FVector2D> ActiveBlocks;
	TArray<FVector2D> CoordOffset = { FVector2D(0, 1), FVector2D(1, 0),
									  FVector2D(0, -1), FVector2D(-1, 0),
									  FVector2D(1, 1), FVector2D(-1, 1),
									  FVector2D(1, -1), FVector2D(-1, -1) };

	// Update timer
	FTimerHandle UpdateTimer;

public:
	/** Returns DummyRoot subobject **/
	FORCEINLINE class USceneComponent* GetDummyRoot() const { return DummyRoot; }
};



