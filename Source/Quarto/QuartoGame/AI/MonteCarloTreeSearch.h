#pragma once
#include "Common/UnrealCommon.h"
#include "QuartoGame/QuartoData.h"
#include <tuple>
#include "Runnable.h"
#include "ScopedEvent.h"
#include "ThreadSafeBool.h"

namespace ai
{
	namespace mcts
	{
		namespace internal
		{
			class MCTSThread;
		}

		class MonteCarloTreeSearch
		{
		public:
			MonteCarloTreeSearch(brFloat maxMoveSearchTimeInSeconds, brFloat maxOpponentTokenSearchTimeInSeconds);
			~MonteCarloTreeSearch();

			void FindNextOpponentToken(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId) const;
			void FindNextMove(QuartoTokenData const& token, QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId) const;
			brBool IsLookingForNextMove() const;
			brBool IsLookingForNextOpponentToken() const;
			brBool HasFoundNextMove() const;
			brBool HasFoundNextOpponentToken() const;
			QuartoBoardSlotCoordinates GetNextMoveCoordinates() const;
			QuartoTokenData GetNextOpponentToken() const;
			
		protected:
			internal::MCTSThread* m_threadWorker = nullptr;
		};

		namespace internal
		{
			struct State
			{
				TArray<State> GetAllPossibleStates(QuartoTokenData const* token) const;
				void RandomPlay();

				//Helper
				void ReplacePlayerIdWithUnused(PlayerId id1, PlayerId id2);

				QuartoBoardData BoardData;
				brU32 VisitCount = 0;
				brS32 WinScore = 0;
				PlayerId PlayerId = 0;
			};

			struct Node
			{
				Node& GetChildWithHighestScore();
				Node& GetRandomChild();

				State State;
				Node* Parent = nullptr;
				TArray<Node> Children;
			};

			//https://wiki.unrealengine.com/MultiThreading_and_synchronization_Guide
			class MCTSThread : public FRunnable
			{
			public:
				MCTSThread(brFloat maxMoveSearchTimeInSeconds, brFloat maxOpponentTokenSearchTimeInSeconds);
				~MCTSThread();
				
				uint32 Run() override;
				void Stop() override;
				brBool IsProcessingAnyRequest() const;

				void RequestNextMove(QuartoTokenData const& token, QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId);
				void RequestNextOpponentToken(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId);

				void SearchNextMove();
				void SearchNextOpponentToken();
				
				brBool IsMoveRequestFinished() const { return m_moveRequest.IsProcessed && m_moveRequest.IsMoveFound; }
				brBool IsOpponentTokenRequestFinished() const { return m_opponentTokenRequest.IsProcessed && m_opponentTokenRequest.IsTokenFound; }
				QuartoTokenData ConsumeRequestResultOpponentToken();
				QuartoBoardSlotCoordinates ConsumeRequestResultNextMove();

			protected:
				void PauseThread();
				void ContinueThread();
				
				QuartoBoardData SearchNextDraw(brFloat maxSearchTime, QuartoBoardData const* boardData, QuartoTokenData const* tokenData, PlayerId playerId, PlayerId opponentId, brBool negate) const;
				
				// Selects the most promising node outgoing from this node
				static Node* Select(Node* node);
				// Expands the given node with new possible nodes
				static void Expand(Node* node, PlayerId playerId, PlayerId opponentId, QuartoTokenData const* token);
				// Simulates a random play and returns the winner
				static PlayerId Simulate(Node* node, PlayerId playerId, PlayerId opponentId, brBool negate);
				// Backpropagates the results
				static void BackPropagate(Node* node, PlayerId playerId, brBool negate);

				static Node* FindBestNodeWithUct(Node* node);

			protected:
				//Thread to run the worker FRunnable on
				FRunnableThread* m_thread;
				FEvent* m_semaphore;
				FCriticalSection m_mutex;
				FThreadSafeBool m_kill;
				FThreadSafeBool m_pause;

				brFloat m_maxMoveSearchTimeInSeconds;
				brFloat m_maxOpponentTokenSearchTimeInSeconds;

				struct
				{
					QuartoBoardData BoardData;
					QuartoTokenData TokenData;
					QuartoBoardSlotCoordinates ResultMove;
					::PlayerId PlayerId;
					::PlayerId OpponentId;
					FThreadSafeBool IsMoveFound;
					FThreadSafeBool IsProcessed;
				} m_moveRequest;

				struct
				{
					QuartoBoardData BoardData;
					QuartoTokenData ResultToken;
					::PlayerId PlayerId;
					::PlayerId OpponentId;
					FThreadSafeBool IsTokenFound;
					FThreadSafeBool IsProcessed;
				} m_opponentTokenRequest;
			};
		}
	}
}