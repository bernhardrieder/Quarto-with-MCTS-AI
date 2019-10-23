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

TArray<brU32> QuartoBoardData::GetEmptySlotIndices() const
{
	TArray<brU32> freeSlotIndices;
	for (brU32 i = 0; i < QUARTO_BOARD_AVAILABLE_SLOTS; ++i)
	{
		if (!m_tokensOnBoardGrid[i].IsValid())
		{
			freeSlotIndices.Push(i);
		}
	}
	return freeSlotIndices;
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

QuartoBoardData::GameStatus QuartoBoardData::GetStatus() const
{
	return GameStatus::End;
}

void QuartoBoardData::SetTokenOnBoard(brU32 slotIndex, QuartoTokenData const& token)
{
	if(slotIndex < QUARTO_BOARD_AVAILABLE_SLOTS)
	{
		m_tokensOnBoardGrid[slotIndex] = token;
	}
}

void QuartoBoardData::SetTokenOnBoard(brU32 slotX, brU32 slotY, QuartoTokenData const& token)
{
	if(slotX < QUARTO_BOARD_SIZE_X && slotY < QUARTO_BOARD_SIZE_Y)
	{
		SetTokenOnBoard(slotY * 4 + slotX, token);
	}
}
