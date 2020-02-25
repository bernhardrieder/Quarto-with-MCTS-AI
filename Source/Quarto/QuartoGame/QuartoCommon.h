#pragma once

#include "Common/UnrealCommon.h"

#define QUARTO_NUM_OF_PLAYERS 2
#define QUARTO_BOARD_SIZE_X 4
#define QUARTO_BOARD_SIZE_Y 4
#define QUARTO_BOARD_AVAILABLE_SLOTS 16

#define GETENUMSTRING(etype, evalue) ( (FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(etype), true)->GetEnumName((int32)evalue) : FString("Invalid - are you sure enum uses UENUM() macro?") )

UENUM(BlueprintType)
enum class EQuartoPlayerType : uint8 /*brU8 -> UE header tool doesn't like it*/
{
	Human 	UMETA(DisplayName = "Human"),
	NPC		UMETA(DisplayName = "NPC")
};

UENUM(BlueprintType)
enum class EQuartoTokenColor : uint8 /*brU8 -> UE header tool doesn't like it*/
{
	Color1 = 0x01 	UMETA(DisplayName = "Color1"),
	Color2 = 0x02	UMETA(DisplayName = "Color2")
};

UENUM(BlueprintType)
enum class EQuartoTokenProperties : uint8 /*brU8 -> UE header tool doesn't like it*/
{
	Quadratic = 0x01	UMETA(DisplayName = "Quadratic"),
	Round = 0x02 	UMETA(DisplayName = "Round"),
	Small = 0x04	UMETA(DisplayName = "Small"),
	Tall = 0x08	UMETA(DisplayName = "Tall"),
	Hole = 0x10	UMETA(DisplayName = "Hole"),
	Filled = 0x20	UMETA(DisplayName = "Filled")
};