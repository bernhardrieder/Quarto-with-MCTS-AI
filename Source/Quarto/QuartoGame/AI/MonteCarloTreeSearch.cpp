#include "MonteCarloTreeSearch.h"

TArray<MonteCarloTreeSearch::State> MonteCarloTreeSearch::State::GetAllPossibleStates() const
{
	TArray<State> states;
	for(auto const& slotCoordinates : BoardData.GetEmptySlotCoordinates())
	{
		for(auto token : BoardData.GetFreeTokens())
		{
			State newState;
			newState.BoardData = BoardData;
			newState.BoardData.SetTokenOnBoard(slotCoordinates, token);
			states.Add(newState);
		}
	}
	return states;
}

void MonteCarloTreeSearch::State::RandomPlay()
{
	auto const emptySlotCoordinates = BoardData.GetEmptySlotCoordinates();
	auto const freeTokens = BoardData.GetFreeTokens();

	if(emptySlotCoordinates.Num() == 0 || freeTokens.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("MCTS: Can't play a random play!"));
		return;
	}

	brU32 const randomSlotIdx = FMath::RandRange(0, emptySlotCoordinates.Num()-1);
	brU32 const randomTokenIdx = FMath::RandRange(0, freeTokens.Num()-1);
	BoardData.SetTokenOnBoard(emptySlotCoordinates[randomSlotIdx], freeTokens[randomTokenIdx]);
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

std::tuple<QuartoTokenData, QuartoBoardSlotCoordinates> MonteCarloTreeSearch::FindNextMove(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId)
{
	Node root;
	root.State.BoardData = currentBoard;
	root.State.PlayerId = opponentId;

	// can be replaced by time 
	for(brU32 sim = 0; sim < 10000; ++sim)
	{
		Node* promisingNode = Select(&root);
		if(promisingNode && promisingNode->State.BoardData.GetStatus() == QuartoBoardData::GameStatus::InProgress)
		{
			Expand(promisingNode, playerId, opponentId);
		}
		Node* nodeToExplore = promisingNode;
		if(promisingNode && promisingNode->Children.Num() > 0)
		{
			nodeToExplore = &promisingNode->GetRandomChild();
		}
		PlayerId const winnerId = Simulate(nodeToExplore, playerId, opponentId);
		BackPropagate(nodeToExplore, winnerId);
	}

	auto oldEmptySlotCoordinates = currentBoard.GetEmptySlotCoordinates();
	auto oldFreeTokens = currentBoard.GetFreeTokens();
	
	auto& winnerBoard = root.GetChildWithHighestScore().State.BoardData;
	for(auto& slotCoordinates : winnerBoard.GetEmptySlotCoordinates())
	{
		oldEmptySlotCoordinates.Remove(slotCoordinates);
	}
	for(auto& token : winnerBoard.GetFreeTokens())
	{
		oldFreeTokens.Remove(token);
	}

	if(oldFreeTokens.Num() < 1 || oldEmptySlotCoordinates.Num() < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: There are no found tokens or slotcoordinates found for the Monte Carlo Tree Search move search!!"));
	}
	return std::make_tuple(oldFreeTokens[0], oldEmptySlotCoordinates[0]);
}

MonteCarloTreeSearch::Node* MonteCarloTreeSearch::Select(Node* node)
{
	Node* result = node;
	while (result && result->Children.Num() > 0)
	{
		result = FindBestNodeWithUct(result);
	}
	return result;
}

void MonteCarloTreeSearch::Expand(Node* node, PlayerId playerId, PlayerId opponentId)
{
	if(!node)
	{
		return;
	}
	
	TArray<State> possibleStates = node->State.GetAllPossibleStates();
	for(State& state : possibleStates)
	{
		Node child;
		child.State = state;
		child.State.PlayerId = node->State.PlayerId;
		child.State.ReplacePlayerIdWithUnused(playerId, opponentId);
		child.Parent = node;
		node->Children.Add(child);
	}
}

MonteCarloTreeSearch::PlayerId MonteCarloTreeSearch::Simulate(Node* node, PlayerId playerId, PlayerId opponentId)
{
	if (!node)
	{
		return 0;
	}
	
	auto status = node->State.BoardData.GetStatus();
	if(status == QuartoBoardData::GameStatus::End 
		&& playerId != node->State.PlayerId)
	{
		if(node->Parent)
		{
			node->Parent->State.WinScore = INT_MIN;
		}
		return node->State.PlayerId;
	}

	State tmpState = node->State;
	while(status == QuartoBoardData::GameStatus::InProgress)
	{
		tmpState.ReplacePlayerIdWithUnused(playerId, opponentId);
		tmpState.RandomPlay();
		status = tmpState.BoardData.GetStatus();
	}
	
	return tmpState.PlayerId;
}

void MonteCarloTreeSearch::BackPropagate(Node* node, PlayerId playerId)
{
	Node* tmpNode = node;
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

MonteCarloTreeSearch::Node* MonteCarloTreeSearch::FindBestNodeWithUct(Node* node)
{
	if(!node)
	{
		return nullptr;
	}
	
	static auto uctValueFct = [](brU32 totalVisit, brFloat nodeWinScore, brU32 nodeVisit) -> brFloat
	{
		static brFloat explorationParam = 1.41f; // sqrt(2)

		if (nodeVisit == 0)
		{
			return brFloatMax;
		}

		return (nodeWinScore / nodeVisit) + explorationParam * FMath::Sqrt(FMath::Loge(totalVisit) / static_cast<brFloat>(nodeVisit));
	};

	Node* bestNode = node;
	brU32 const parentVisit = node->State.VisitCount;
	brFloat highestUctValue = 0.f;
	for(Node& childNode : node->Children)
	{
		brFloat const uctValue = uctValueFct(parentVisit, childNode.State.WinScore, childNode.State.VisitCount);
		if(uctValue > highestUctValue)
		{
			bestNode = &childNode;
			highestUctValue = uctValue;
		}
	}
	
	return bestNode;
}
