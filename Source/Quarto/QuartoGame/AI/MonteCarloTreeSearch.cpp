#include "MonteCarloTreeSearch.h"

TArray<MonteCarloTreeSearch::State> MonteCarloTreeSearch::State::GetAllPossibleStates() const
{
	TArray<State> states;
	for(brU32 slotIndex : BoardData.GetEmptySlotIndices())
	{
		for(auto token : BoardData.GetFreeTokens())
		{
			State newState;
			newState.BoardData = BoardData;
			newState.BoardData.SetTokenOnBoard(slotIndex, token);
			states.Add(newState);
		}
	}
	return states;
}

void MonteCarloTreeSearch::State::RandomPlay()
{
	auto const emptySlotIndices = BoardData.GetEmptySlotIndices();
	auto const freeTokens = BoardData.GetFreeTokens();
	brU32 const randomSlotIdx = FMath::RandRange(0, emptySlotIndices.Num()-1);
	brU32 const randomTokenIdx = FMath::RandRange(0, freeTokens.Num()-1);
	BoardData.SetTokenOnBoard(emptySlotIndices[randomSlotIdx], freeTokens[randomTokenIdx]);
}

void MonteCarloTreeSearch::State::ReplacePlayerIdWithUnused(MonteCarloTreeSearch::PlayerId id1,
	MonteCarloTreeSearch::PlayerId id2)
{
	if(PlayerId == id1)
	{
		PlayerId = id2;
	}
	else if(PlayerId == id2)
	{
		PlayerId = id1;
	}
}

MonteCarloTreeSearch::Node& MonteCarloTreeSearch::Node::GetChildWithHighestScore()
{
	if(Children.Num() == 0)
	{
		return *this;
	}
	
	Node* bestChild = nullptr;
	for(Node& child : Children)
	{
		if(!bestChild ||
			(bestChild && child.State.VisitCount > bestChild->State.VisitCount))
		{
			bestChild = &child;
		}
	}
	return *bestChild;
}

MonteCarloTreeSearch::Node& MonteCarloTreeSearch::Node::GetRandomChild()
{
	return Children[FMath::RandRange(0, Children.Num() - 1)];
}

std::tuple<QuartoTokenData, brU32> MonteCarloTreeSearch::FindNextMove(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId)
{
	Node root;
	root.State.BoardData = currentBoard;
	root.State.PlayerId = opponentId;

	// can be replaced by time 
	for(brU32 sim = 0; sim < 100; ++sim)
	{
		Node& promisingNode = Select(root);
		if(promisingNode.State.BoardData.GetStatus() == QuartoBoardData::GameStatus::InProgress)
		{
			Expand(promisingNode, playerId, opponentId);
		}
		Node& nodeToExplore = promisingNode;
		if(promisingNode.Children.Num() > 0)
		{
			nodeToExplore = promisingNode.GetRandomChild();
		}
		PlayerId const winnerId = Simulate(nodeToExplore, playerId, opponentId);
		BackPropagate(nodeToExplore, winnerId);
	}

	auto oldEmptySlots = currentBoard.GetEmptySlotIndices();
	auto oldFreeTokens = currentBoard.GetFreeTokens();
	
	auto& winnerBoard = root.GetChildWithHighestScore().State.BoardData;
	for(auto& slotIndex : winnerBoard.GetEmptySlotIndices())
	{
		oldEmptySlots.Remove(slotIndex);
	}
	for(auto& token : winnerBoard.GetFreeTokens())
	{
		oldFreeTokens.Remove(token);
	}
	
	return std::make_tuple(oldFreeTokens[0], oldEmptySlots[0]);
}

MonteCarloTreeSearch::Node& MonteCarloTreeSearch::Select(Node& node)
{
	Node& result = node;
	while (result.Children.Num() > 0)
	{
		result = FindBestNodeWithUct(result);
	}
	return result;
}

void MonteCarloTreeSearch::Expand(Node& node, PlayerId playerId, PlayerId opponentId)
{
	TArray<State> possibleStates = node.State.GetAllPossibleStates();
	for(State& state : possibleStates)
	{
		Node newNode;
		newNode.State = state;
		newNode.State.PlayerId = node.State.PlayerId;
		newNode.State.ReplacePlayerIdWithUnused(playerId, opponentId);
		newNode.Parent = &node;
		node.Children.Add(newNode);
	}
}

MonteCarloTreeSearch::PlayerId MonteCarloTreeSearch::Simulate(Node& node, PlayerId playerId, PlayerId opponentId)
{
	auto status = node.State.BoardData.GetStatus();
	if(status == QuartoBoardData::GameStatus::End 
		&& playerId != node.State.PlayerId)
	{
		node.Parent->State.WinScore = INT_MIN;
		return node.State.PlayerId;
	}

	State tmpState = node.State;
	while(status == QuartoBoardData::GameStatus::InProgress)
	{
		tmpState.ReplacePlayerIdWithUnused(playerId, opponentId);
		tmpState.RandomPlay();
		status = tmpState.BoardData.GetStatus();
	}
	
	return tmpState.PlayerId;
}

void MonteCarloTreeSearch::BackPropagate(Node& node, PlayerId playerId)
{
	Node* tmpNode = &node;
	while(tmpNode)
	{
		State& state = tmpNode->State;
		++(state.VisitCount);
		if(state.PlayerId == playerId)
		{
			state.WinScore += 10;
		}
		tmpNode = tmpNode->Parent;
	}
}

MonteCarloTreeSearch::Node& MonteCarloTreeSearch::FindBestNodeWithUct(Node& node)
{
	static auto uctValueFct = [](brU32 totalVisit, brFloat nodeWinScore, brU32 nodeVisit) -> brFloat
	{
		static brFloat explorationParam = 1.41f; // sqrt(2)

		if (nodeVisit == 0)
		{
			return brFloatMax;
		}

		return (nodeWinScore / nodeVisit) + explorationParam * FMath::Sqrt(FMath::Loge(totalVisit) / static_cast<brFloat>(nodeVisit));
	};

	Node& bestNode = node;
	brU32 const parentVisit = node.State.VisitCount;
	brFloat highestUctValue = 0.f;
	for(Node& childNode : node.Children)
	{
		brFloat const uctValue = uctValueFct(parentVisit, childNode.State.WinScore, childNode.State.VisitCount);
		if(uctValue > highestUctValue)
		{
			bestNode = childNode;
			highestUctValue = uctValue;
		}
	}
	
	return bestNode;
}
