#pragma once
#include "Common/UnrealCommon.h"
#include "QuartoGame/QuartoData.h"
#include <tuple>

//https://www.baeldung.com/java-monte-carlo-tree-search

class MonteCarloTreeSearch
{
	using PlayerId = brU32;
	struct State
	{
		TArray<State> GetAllPossibleStates() const;
		void RandomPlay();

		//Helper
		void ReplacePlayerIdWithUnused(PlayerId id1, PlayerId id2);

		QuartoBoardData BoardData;
		brU32 VisitCount;
		brS32 WinScore;
		PlayerId PlayerId;
	};

	struct Node
	{
		Node& GetChildWithHighestScore();
		Node& GetRandomChild();
		
		State State;
		Node* Parent;
		TArray<Node> Children;
	};
	
public:
	static std::tuple<QuartoTokenData, brU32> FindNextMove(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId);
	
protected:
	// Selects the most promising node outgoing from this node
	static Node& Select(Node& node);
	// Expands the given node with new possible nodes
	static void Expand(Node& node, PlayerId playerId, PlayerId opponentId);
	// Simulates a random play
	static PlayerId Simulate(Node& node, PlayerId playerId, PlayerId opponentId);
	// Backpropagate 
	static void BackPropagate(Node& node, PlayerId playerId);

	static Node& FindBestNodeWithUct(Node& node);

};