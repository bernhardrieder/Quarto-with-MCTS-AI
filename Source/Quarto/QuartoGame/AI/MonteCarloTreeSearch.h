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
			MonteCarloTreeSearch(brFloat maxMoveSearchTime);
			~MonteCarloTreeSearch();

			void FindNextMove(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId) const;
			brBool IsLookingForNextMove() const;
			brBool HasFoundNextMove() const;
			std::tuple<QuartoTokenData, QuartoBoardSlotCoordinates> GetNextMove() const;

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
				MCTSThread(brFloat maxMoveSearchTime);
				~MCTSThread();
				
				uint32 Run() override;
				void Stop() override;
				brBool IsProcessingRequest() const;

				void RequestNextMove(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId);
				brBool IsRequestFinished() const { return m_request.IsFinished; }
				std::tuple<QuartoTokenData, QuartoBoardSlotCoordinates> ConsumeRequestResult();

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

				brFloat m_maxSearchTimeInSeconds;

				struct
				{
					std::tuple<QuartoTokenData, QuartoBoardSlotCoordinates> Result;
					QuartoBoardData BoardData;
					::PlayerId PlayerId;
					::PlayerId OpponentId;
					FThreadSafeBool IsFinished;
				} m_request;
			};
		}
	}
}