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

			void FindNextMove(QuartoTokenData token, QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId) const;
			brBool IsLookingForNextMove() const;
			brBool HasFoundNextMove() const;
			QuartoBoardSlotCoordinates GetNextMoveCoordinates() const;
			QuartoTokenData GetNextOpponentToken() const;
			
		protected:
			internal::MCTSThread* m_threadWorker = nullptr;
		};

		namespace internal
		{
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
				brBool IsProcessingRequest() const;

				void RequestNextMoveAndOpponentToken(QuartoTokenData token, QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId);

				void SearchNextMove();
				void SearchNextOpponentToken();
				
				brBool IsRequestFinished() const { return m_request.IsMoveFound && m_request.IsTokenFound; }
				QuartoTokenData ConsumeRequestResultToken();
				QuartoBoardSlotCoordinates ConsumeRequestResultMove();

			protected:
				void PauseThread();
				void ContinueThread();
				
				// Selects the most promising node outgoing from this node
				static Node* Select(Node* node);
				// Expands the given node with new possible nodes
				static void Expand(Node* node, PlayerId playerId, PlayerId opponentId);
				// Simulates a random play and returns the winner
				static PlayerId Simulate(Node* node, PlayerId playerId, PlayerId opponentId);
				// Backpropagates the results
				static void BackPropagate(Node* node, PlayerId playerId);

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
					QuartoTokenData ResultOpponentToken;
					QuartoBoardSlotCoordinates ResultMove;
					QuartoBoardData BoardData;
					::PlayerId PlayerId;
					::PlayerId OpponentId;
					FThreadSafeBool IsMoveFound;
					FThreadSafeBool IsTokenFound;
					QuartoTokenData TokenData;
				} m_request;
			};
		}
	}
}