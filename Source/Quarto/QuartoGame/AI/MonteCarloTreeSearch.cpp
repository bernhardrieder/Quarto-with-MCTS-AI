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

QuartoBoardData MonteCarloTreeSearch::FindNextMove(QuartoBoardData& currentBoard, PlayerId playerId)
{
	Node root;
	root.State.BoardData = currentBoard;
	root.State.PlayerId = m_playerOpponent[playerId];

	// can be replaced by time 
	for(brU32 sim = 0; sim < 100; ++sim)
	{
		Node& promisingNode = Select(root);
		if(promisingNode.State.BoardData.GetStatus() == QuartoBoardData::GameStatus::InProgress)
		{
			Expand(promisingNode);
		}
		Node& nodeToExplore = promisingNode;
		if(promisingNode.Children.Num() > 0)
		{
			nodeToExplore = promisingNode.GetRandomChild();
		}
		PlayerId const winnerId = Simulate(nodeToExplore, playerId);
		BackPropagate(nodeToExplore, winnerId);
	}
	
	return root.GetChildWithHighestScore().State.BoardData;
}

void MonteCarloTreeSearch::RegisterPlayerOpponent(PlayerId playerId, PlayerId opponentId)
{
	m_playerOpponent[playerId] = opponentId;
}

void MonteCarloTreeSearch::RemovePlayerOpponent(PlayerId playerId, PlayerId opponentId)
{
	m_playerOpponent.Remove(playerId);
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

void MonteCarloTreeSearch::Expand(Node& node)
{
	TArray<State> possibleStates = node.State.GetAllPossibleStates();
	for(State& state : possibleStates)
	{
		Node newNode;
		newNode.State = state;
		newNode.State.PlayerId = m_playerOpponent[node.State.PlayerId];
		newNode.Parent = &node;
		node.Children.Add(newNode);
	}
}

MonteCarloTreeSearch::PlayerId MonteCarloTreeSearch::Simulate(Node& node, PlayerId playerId)
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
		tmpState.PlayerId = m_playerOpponent[tmpState.PlayerId];
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
