#pragma once
#include "Common/UnrealCommon.h"
#include "QuartoGame/QuartoData.h"

//https://www.baeldung.com/java-monte-carlo-tree-search

class MonteCarloTreeSearch
{
	using PlayerId = brU32;
	struct State
	{
		TArray<State> GetAllPossibleStates() const;
		void RandomPlay();

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
	//MonteCarloTreeSearch() = default;

	QuartoBoardData FindNextMove(QuartoBoardData& currentBoard, PlayerId playerId);
	void RegisterPlayerOpponent(PlayerId playerId, PlayerId opponentId);
	void RemovePlayerOpponent(PlayerId playerId, PlayerId opponentId);
	
protected:
	// Selects the most promising node outgoing from this node
	static Node& Select(Node& node);
	// Expands the given node with new possible nodes
	void Expand(Node& node);
	// Simulates a random play
	PlayerId Simulate(Node& node, PlayerId playerId);
	// Backpropagate 
	static void BackPropagate(Node& node, PlayerId playerId);

	static Node& FindBestNodeWithUct(Node& node);

	TMap<PlayerId, PlayerId> m_playerOpponent;
};