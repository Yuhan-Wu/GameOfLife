// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GameOfLifeBlockGrid.h"
#include "GameOfLifeBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextRenderComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "PuzzleBlockGrid"

AGameOfLifeBlockGrid::AGameOfLifeBlockGrid()
{
	// Create dummy root scene component
	DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Dummy0"));
	RootComponent = DummyRoot;

	SpeedText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ScoreText0"));
	SpeedText->SetRelativeLocation(FVector(-200.f, 0.f, 2000.f));
	SpeedText->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	SpeedText->SetText(FText::Format(LOCTEXT("SpeedFmt", "Speed: {0}"), FText::AsNumber(1.0)));
	SpeedText->SetupAttachment(DummyRoot);

	PauseText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("PauseText0"));
	PauseText->SetRelativeLocation(FVector(-200.f, 0.f, 2000.f));
	PauseText->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	PauseText->SetText("Pause");
	PauseText->SetupAttachment(DummyRoot);

	// Set defaults
	Size = 3;
	BlockSpacing = 300.f;

	// Set running configurations
	IsRunning = false;
	RestTime = 2;
	CurSpeedOption = 2;
}


void AGameOfLifeBlockGrid::BeginPlay()
{
	Super::BeginPlay();

	// Enable input
	EnableInput(UGameplayStatics::GetPlayerController(this, 0));
	InputComponent->BindAction("ToggleStart", IE_Pressed, this, &AGameOfLifeBlockGrid::ToggleStart);
	InputComponent->BindAction("SpeedUp", IE_Pressed, this, &AGameOfLifeBlockGrid::SpeedUp);
	InputComponent->BindAction("SlowDown", IE_Pressed, this, &AGameOfLifeBlockGrid::SlowDown);
	InputComponent->BindAction("Restart", IE_Pressed, this, &AGameOfLifeBlockGrid::Restart);

	// Number of blocks
	const int32 NumBlocks = Size * Size;

	// Loop to spawn each block
	for(int32 BlockIndex=0; BlockIndex<NumBlocks; BlockIndex++)
	{
		const float XOffset = (BlockIndex / Size) * BlockSpacing; // Divide by dimension
		const float YOffset = (BlockIndex % Size) * BlockSpacing; // Modulo gives remainder

		// Make position vector, offset from Grid location
		const FVector BlockLocation = FVector(XOffset, YOffset, 0.f) + GetActorLocation();

		// Spawn a block
		AGameOfLifeBlock* NewBlock = GetWorld()->SpawnActor<AGameOfLifeBlock>(BlockLocation, FRotator(0,0,0));
		NewBlock->SetCoordinate(BlockIndex / Size, BlockIndex % Size);
		GridInfo.Push(NewBlock);

		// Tell the block about its owner
		if (NewBlock != nullptr)
		{
			NewBlock->OwningGrid = this;
		}
	}
}

void AGameOfLifeBlockGrid::RegisterHighlightBlock(FVector2D Coordinate) {
	if (!ActiveBlocks.Contains(Coordinate))
		ActiveBlocks.Push(Coordinate);
	RegisterNeighbor(Coordinate, ActiveBlocks);
}

void AGameOfLifeBlockGrid::RegisterHighlightBlock(FVector2D Coordinate, TArray<FVector2D>& TempList) {
	if(!ActiveBlocks.Contains(Coordinate) && !TempList.Contains(Coordinate))
		TempList.Push(Coordinate);
	RegisterNeighbor(Coordinate, TempList);
}

void AGameOfLifeBlockGrid::RegisterNeighbor(FVector2D Coordinate, TArray<FVector2D>& TempList) {
	// UE_LOG(LogTemp, Log, TEXT("Register neighbor"));

	for (int j = 0; j < CoordOffset.Num(); j++) {
		FVector2D neighbor = Coordinate + CoordOffset[j];
		if (neighbor.X > 0 && neighbor.X < Size &&
			neighbor.Y > 0 && neighbor.Y < Size) {
			if (!ActiveBlocks.Contains(neighbor) && !TempList.Contains(neighbor)) {
				TempList.Push(neighbor);
			}
		}
	}
}

void AGameOfLifeBlockGrid::ToggleStart() {
	if (IsRunning) {
		IsRunning = false;
		ClearTimer();
		PauseText->SetText("Pause");
	}
	else {
		IsRunning = true;
		SetUpTimer();
		PauseText->SetText("");
	}
	UE_LOG(LogClass, Log, TEXT("Space is pressed."));
}

void AGameOfLifeBlockGrid::TriggerGridUpdate() {

	TArray<FVector2D> ToBeMerged;
	TArray<AGameOfLifeBlock*> NewUnhighlight;
	TArray<AGameOfLifeBlock*> NewHighlight;

	int index = 0;
	while (index < ActiveBlocks.Num()) {
		FVector2D curPos = ActiveBlocks[index];
		int livinNeighbor = 0;
		for (int j = 0; j < CoordOffset.Num(); j++) {
			FVector2D neighbor = curPos + CoordOffset[j];
			if (neighbor.X > 0 && neighbor.X < Size &&
				neighbor.Y > 0 && neighbor.Y < Size) {
				if (GridInfo[neighbor.X * Size + neighbor.Y]->bIsActive) {
					livinNeighbor++;
				}
			}
		}

		bool curState = GridInfo[curPos.X * Size + curPos.Y]->bIsActive;
		int curIndex = curPos.X * Size + curPos.Y;
		// Keep a record
		if (livinNeighbor < 2) {
			if (curState) {
				// UE_LOG(LogClass, Log, TEXT("Add unhighlight: %s"), *GridInfo[index]->GetCoordinate().ToString());
				NewUnhighlight.Push(GridInfo[curIndex]);
			}	

			if (livinNeighbor == 0)
				// No chance for updating
				ActiveBlocks.RemoveAt(index);
			else
				index++;
		}
		else if (curState && livinNeighbor > 3) {
			NewUnhighlight.Push(GridInfo[curIndex]);
			index++;
		}
		else if (!curState && livinNeighbor == 3) {
			NewHighlight.Push(GridInfo[curIndex]);
			RegisterHighlightBlock(ActiveBlocks[index], ToBeMerged);
			index++;
		}
		else {
			index++;
		}
	}

	// Merge 2 lists
	ActiveBlocks.Append(ToBeMerged);
	// Update block
	for (AGameOfLifeBlock* item : NewUnhighlight) {
		// UE_LOG(LogClass, Log, TEXT("Unhighlight: %s"), *item->GetCoordinate().ToString());
		item->UnsetHighlight();
	}
	for (AGameOfLifeBlock* item : NewHighlight) {
		item->SetHighlight();
	}
}

void AGameOfLifeBlockGrid::SetUpTimer() {
	GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this, &AGameOfLifeBlockGrid::TriggerGridUpdate, RestTime / SpeedOptions[CurSpeedOption], true);
}

void AGameOfLifeBlockGrid::ClearTimer() {
	GetWorld()->GetTimerManager().ClearTimer(UpdateTimer);
}

void AGameOfLifeBlockGrid::SpeedUp() {
	if (CurSpeedOption + 1 < SpeedOptions.Num()) {
		CurSpeedOption++;
	}
	UpdateSpeedText();
	ClearTimer();
	if (IsRunning)
		SetUpTimer();
}

void AGameOfLifeBlockGrid::SlowDown() {
	if (CurSpeedOption - 1 >= 0) {
		CurSpeedOption--;
	}
	UpdateSpeedText();
	ClearTimer();
	if (IsRunning)
		SetUpTimer();
}

void AGameOfLifeBlockGrid::UpdateSpeedText() {
	SpeedText->SetText(FText::Format(LOCTEXT("SpeedFmt", "Speed: {0}"), FText::AsNumber(SpeedOptions[CurSpeedOption])));
}

void AGameOfLifeBlockGrid::Restart() {
	// Pause
	IsRunning = false;
	ClearTimer();
	PauseText->SetText("Pause");

	// Reset
	for (int i = 0; i < ActiveBlocks.Num(); i++) {
		GridInfo[ActiveBlocks[i].X * Size + ActiveBlocks[i].Y]->UnsetHighlight();
	}
	ActiveBlocks.Empty();
}

#undef LOCTEXT_NAMESPACE
