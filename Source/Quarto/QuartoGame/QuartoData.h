#pragma once

#include "Common/UnrealCommon.h"
#include "QuartoCommon.h"

struct QuartoTokenData
{
	QuartoTokenData()
		: QuartoTokenData(EQuartoTokenColor::Color1, {}) {};
	QuartoTokenData(EQuartoTokenColor color, EQuartoTokenProperties property1, EQuartoTokenProperties property2, EQuartoTokenProperties property3)
		: QuartoTokenData(color, { property1, property2, property3 }) {}
	QuartoTokenData(EQuartoTokenColor color, TArray<EQuartoTokenProperties> properties);

	TArray<EQuartoTokenProperties> GetProperties() const { return m_properties; }
	bool HasAtLeastOneMatchingProperty(QuartoTokenData& other) const { return (m_propertiesBitmask & other.m_propertiesBitmask) > 0; }
	brU32 GetPropertiesBitMask() const { return m_propertiesBitmask; }

	EQuartoTokenColor GetColor() const { return m_color; }
	brU32 GetColorBitMask() const { return static_cast<brU32>(m_color); }

	brBool IsValid() const { return m_propertiesBitmask > 0; }
	void Invalidate() { m_propertiesBitmask = 0; }

public:
	static TArray<QuartoTokenData> PossiblePermutations;

private:
	EQuartoTokenColor m_color;
	TArray<EQuartoTokenProperties> m_properties;
	brU32 m_propertiesBitmask;
};

struct QuartoBoardData
{
	brU32 GetNumberOfFreeSlots() const;
	TArray<brU32> GetEmptySlotIndices() const;
	void Reset();

	QuartoTokenData m_tokensOnBoardGrid[QUARTO_BOARD_AVAILABLE_SLOTS]; //xDim, yDim = 4
};