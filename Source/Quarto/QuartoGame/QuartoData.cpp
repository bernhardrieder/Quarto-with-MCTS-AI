#include "QuartoData.h"

TArray<QuartoTokenData> QuartoTokenData::s_possiblePermutations =
{
	QuartoTokenData(EQuartoTokenColor::Color1, EQuartoTokenProperties::Filled, EQuartoTokenProperties::Quadratic, EQuartoTokenProperties::Small),
	QuartoTokenData(EQuartoTokenColor::Color1, EQuartoTokenProperties::Filled, EQuartoTokenProperties::Quadratic, EQuartoTokenProperties::Tall),
	QuartoTokenData(EQuartoTokenColor::Color1, EQuartoTokenProperties::Hole, EQuartoTokenProperties::Quadratic, EQuartoTokenProperties::Small),
	QuartoTokenData(EQuartoTokenColor::Color1, EQuartoTokenProperties::Hole, EQuartoTokenProperties::Quadratic, EQuartoTokenProperties::Tall),
	QuartoTokenData(EQuartoTokenColor::Color1, EQuartoTokenProperties::Filled, EQuartoTokenProperties::Round, EQuartoTokenProperties::Small),
	QuartoTokenData(EQuartoTokenColor::Color1, EQuartoTokenProperties::Filled, EQuartoTokenProperties::Round, EQuartoTokenProperties::Tall),
	QuartoTokenData(EQuartoTokenColor::Color1, EQuartoTokenProperties::Hole, EQuartoTokenProperties::Round, EQuartoTokenProperties::Small),
	QuartoTokenData(EQuartoTokenColor::Color1, EQuartoTokenProperties::Hole, EQuartoTokenProperties::Round, EQuartoTokenProperties::Tall),
	QuartoTokenData(EQuartoTokenColor::Color2, EQuartoTokenProperties::Filled, EQuartoTokenProperties::Quadratic, EQuartoTokenProperties::Small),
	QuartoTokenData(EQuartoTokenColor::Color2, EQuartoTokenProperties::Filled, EQuartoTokenProperties::Quadratic, EQuartoTokenProperties::Tall),
	QuartoTokenData(EQuartoTokenColor::Color2, EQuartoTokenProperties::Hole, EQuartoTokenProperties::Quadratic, EQuartoTokenProperties::Small),
	QuartoTokenData(EQuartoTokenColor::Color2, EQuartoTokenProperties::Hole, EQuartoTokenProperties::Quadratic, EQuartoTokenProperties::Tall),
	QuartoTokenData(EQuartoTokenColor::Color2, EQuartoTokenProperties::Filled, EQuartoTokenProperties::Round, EQuartoTokenProperties::Small),
	QuartoTokenData(EQuartoTokenColor::Color2, EQuartoTokenProperties::Filled, EQuartoTokenProperties::Round, EQuartoTokenProperties::Tall),
	QuartoTokenData(EQuartoTokenColor::Color2, EQuartoTokenProperties::Hole, EQuartoTokenProperties::Round, EQuartoTokenProperties::Small),
	QuartoTokenData(EQuartoTokenColor::Color2, EQuartoTokenProperties::Hole, EQuartoTokenProperties::Round, EQuartoTokenProperties::Tall),
};

QuartoTokenData::QuartoTokenData(EQuartoTokenColor color, TArray<EQuartoTokenProperties> properties)
	: m_color(color)
	, m_properties(properties)
	, m_propertiesBitmask(0u)
{
	for (EQuartoTokenProperties property : properties)
	{
		m_propertiesBitmask |= static_cast<brS32>(property);
	}
}

bool QuartoTokenData::operator==(QuartoTokenData const& other) const
{
	return this->GetColor() == other.GetColor() && this->GetPropertiesBitMask() == other.GetPropertiesBitMask();
}

bool QuartoBoardSlotCoordinates::operator==(QuartoBoardSlotCoordinates const& other) const
{
	return X == other.X && Y == other.Y;
}

brU32 QuartoBoardData::GetNumberOfFreeSlots() const
{
	brU32 count = 0;
	for (auto& tokenData : m_tokensOnBoardGrid)
	{
		if (tokenData.IsValid())
		{
			++count;
		}
	}
	return count;
}

TArray<QuartoBoardSlotCoordinates> QuartoBoardData::GetEmptySlotCoordinates() const
{
	TArray<QuartoBoardSlotCoordinates> freeSlotCoordinates;
	for (brU32 i = 0; i < QUARTO_BOARD_AVAILABLE_SLOTS; ++i)
	{
		if (!m_tokensOnBoardGrid[i].IsValid())
		{
			freeSlotCoordinates.Push(ConvertIndexToSlotCoordinates(i));
		}
	}
	return freeSlotCoordinates;
}

TArray<QuartoTokenData> QuartoBoardData::GetFreeTokens() const
{
	TArray<QuartoTokenData> tokens = QuartoTokenData::s_possiblePermutations;
	for(auto& token : m_tokensOnBoardGrid)
	{
		if(token.IsValid())
		{
			tokens.Remove(token);
		}
	}
	return tokens;
}

void QuartoBoardData::Reset()
{
	for (auto& tokenData : m_tokensOnBoardGrid)
	{
		tokenData.Invalidate();
	}
}

QuartoBoardSlotCoordinates QuartoBoardData::ConvertIndexToSlotCoordinates(brU32 slotIndex)
{
	return QuartoBoardSlotCoordinates(slotIndex % QUARTO_BOARD_SIZE_Y, slotIndex / QUARTO_BOARD_SIZE_Y);
}

brU32 QuartoBoardData::ConvertCoordinatesToSlotIndex(QuartoBoardSlotCoordinates const& coordinates)
{
	return coordinates.Y * QUARTO_BOARD_SIZE_Y + coordinates.X;
}

QuartoBoardData::GameStatus QuartoBoardData::GetStatus() const
{
	static constexpr brU8 numWinConstellations = 10;
	static brU32 winConstellations[numWinConstellations][4] =
	{
		//vertical
		{0,4,8,12},
		{1,5,9,13},
		{2,6,10,14},
		{3,7,11,15},

		//horizontal
		{0,1,2,3},
		{4,5,6,7},
		{8,9,10,11},
		{12,13,14,15},

		//diagonal
		{0,5,10,15},
		{12,9,6,3}
	};

	for (brU8 y = 0; y < numWinConstellations; ++y)
	{
		brU32* indices = winConstellations[y];

		if (!m_tokensOnBoardGrid[indices[0]].IsValid()
			|| !m_tokensOnBoardGrid[indices[1]].IsValid()
			|| !m_tokensOnBoardGrid[indices[2]].IsValid()
			|| !m_tokensOnBoardGrid[indices[3]].IsValid())
		{
			continue;
		}

		brU32 const matchingPropertiesMask =
			m_tokensOnBoardGrid[indices[0]].GetPropertiesBitMask() &
			m_tokensOnBoardGrid[indices[1]].GetPropertiesBitMask() &
			m_tokensOnBoardGrid[indices[2]].GetPropertiesBitMask() &
			m_tokensOnBoardGrid[indices[3]].GetPropertiesBitMask();

		brU32 const matchingColor =
			m_tokensOnBoardGrid[indices[0]].GetColorBitMask() &
			m_tokensOnBoardGrid[indices[1]].GetColorBitMask() &
			m_tokensOnBoardGrid[indices[2]].GetColorBitMask() &
			m_tokensOnBoardGrid[indices[3]].GetColorBitMask();

		//see EQuartoTokenColor && EQuartoTokenProperties that no value starts at 0
		if (matchingPropertiesMask > 0 || matchingColor > 0)
		{
			return GameStatus::End;
		}
	}

	return GameStatus::InProgress;
}

void QuartoBoardData::SetTokenOnBoard(QuartoBoardSlotCoordinates coordinates, QuartoTokenData const& token)
{
	if (coordinates.AreValid())
	{
		m_tokensOnBoardGrid[ConvertCoordinatesToSlotIndex(coordinates)] = token;
	}
}