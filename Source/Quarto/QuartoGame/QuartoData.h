#pragma once

#include "Quarto/Common/UnrealCommon.h"
#include "Quarto/QuartoGame/QuartoCommon.h"

struct QuartoTokenData
{
	QuartoTokenData()
		: QuartoTokenData(EQuartoTokenColor::Color1, {}) {};
	QuartoTokenData(EQuartoTokenColor color, EQuartoTokenProperties property1, EQuartoTokenProperties property2, EQuartoTokenProperties property3)
		: QuartoTokenData(color, { property1, property2, property3 }) {}
	QuartoTokenData(EQuartoTokenColor color, TArray<EQuartoTokenProperties> properties);

	bool operator==(QuartoTokenData const& other) const;
	
	TArray<EQuartoTokenProperties> GetProperties() const { return m_properties; }
	bool HasAtLeastOneMatchingProperty(QuartoTokenData& other) const { return (m_propertiesBitmask & other.m_propertiesBitmask) > 0; }
	brU32 GetPropertiesBitMask() const { return m_propertiesBitmask; }

	EQuartoTokenColor GetColor() const { return m_color; }
	brU32 GetColorBitMask() const { return static_cast<brU32>(m_color); }

	brBool IsValid() const { return m_propertiesBitmask > 0; }
	void Invalidate() { m_propertiesBitmask = 0; }

public:
	static TArray<QuartoTokenData> s_possiblePermutations;

private:
	EQuartoTokenColor m_color;
	TArray<EQuartoTokenProperties> m_properties;
	brU32 m_propertiesBitmask;
};

struct QuartoBoardSlotCoordinates
{
	QuartoBoardSlotCoordinates() : X(0), Y(0) {}
	QuartoBoardSlotCoordinates(brU32 x, brU32 y) : X(x), Y(y) {}

	bool operator==(QuartoBoardSlotCoordinates const& other) const;
	
	constexpr brBool AreValid() const noexcept { return X < QUARTO_BOARD_SIZE_X && Y < QUARTO_BOARD_SIZE_Y; };

	brU32 X, Y;
};

struct QuartoBoardData
{
	enum class GameStatus
	{
		InProgress,
		End
	};

	brU32 GetNumberOfFreeSlots() const;
	TArray<QuartoBoardSlotCoordinates> GetEmptySlotCoordinates() const;
	TArray<QuartoTokenData> GetFreeTokens() const;
	GameStatus GetStatus() const;

	void SetTokenOnBoard(QuartoBoardSlotCoordinates coordinates, QuartoTokenData const& token);
	void Reset();

private:
	static QuartoBoardSlotCoordinates ConvertIndexToSlotCoordinates(brU32 slotIndex);
	static brU32 ConvertCoordinatesToSlotIndex(QuartoBoardSlotCoordinates const& coordinates);
	
	QuartoTokenData m_tokensOnBoardGrid[QUARTO_BOARD_AVAILABLE_SLOTS]; //xDim, yDim = 4
};
