#pragma once

#define QUARTO_NUM_OF_PLAYERS 2
#define QUARTO_BOARD_SIZE_X 4
#define QUARTO_BOARD_SIZE_Y 4
#define QUARTO_BOARD_AVAILABLE_SLOTS 16

UENUM(BlueprintType)
enum class EQuartoPlayerType : uint8 /*brU8 -> UE header tool doesn't like it*/
{
	Human 	UMETA(DisplayName = "Human"),
	NPC		UMETA(DisplayName = "NPC")
};

UENUM(BlueprintType)
enum class EQuartoTokenColor : uint8 /*brU8 -> UE header tool doesn't like it*/
{
	Undefined	= 0x00	UMETA(DisplayName = "Undefined"),
	Color1		= 0x01 	UMETA(DisplayName = "Color1"),
	Color2		= 0x02	UMETA(DisplayName = "Color2")
};

UENUM(BlueprintType)
enum class EQuartoTokenProperties : uint8 /*brU8 -> UE header tool doesn't like it*/
{
	Undefined	= 0x00	UMETA(DisplayName = "Undefined"),
	Quadratic	= 0x01	UMETA(DisplayName = "Quadratic"),
	Round		= 0x02 	UMETA(DisplayName = "Round"),
	Small		= 0x04	UMETA(DisplayName = "Small"),
	Tall		= 0x08	UMETA(DisplayName = "Tall"),
	Hole		= 0x10	UMETA(DisplayName = "Hole"),
	Filled		= 0x20	UMETA(DisplayName = "Filled")
};