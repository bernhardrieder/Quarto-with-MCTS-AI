#include "MonteCarloTreeSearch.h"
#include "PlatformProcess.h"
#include "RunnableThread.h"

using namespace ai::mcts;

MonteCarloTreeSearch::MonteCarloTreeSearch(brFloat maxMoveSearchTimeInSeconds, brFloat maxOpponentTokenSearchTimeInSeconds)
{
	m_threadWorker = new internal::MCTSThread(maxMoveSearchTimeInSeconds, maxOpponentTokenSearchTimeInSeconds);
}

MonteCarloTreeSearch::~MonteCarloTreeSearch()
{
	delete m_threadWorker;
}

void MonteCarloTreeSearch::FindNextMove(QuartoTokenData token, QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId) const
{
	m_threadWorker->RequestNextMoveAndOpponentToken(token, currentBoard, playerId, opponentId);
}

brBool MonteCarloTreeSearch::IsLookingForNextMove() const
{
	return m_threadWorker->IsProcessingRequest();
}

brBool MonteCarloTreeSearch::HasFoundNextMove() const
{
	return m_threadWorker->IsRequestFinished();
}

QuartoBoardSlotCoordinates MonteCarloTreeSearch::GetNextMoveCoordinates() const
{
	return m_threadWorker->ConsumeRequestResultMove();
}

QuartoTokenData MonteCarloTreeSearch::GetNextOpponentToken() const
{
	return m_threadWorker->ConsumeRequestResultToken();
}


TArray<internal::State> internal::State::GetAllPossibleStates() const
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

void internal::State::RandomPlay()
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

void internal::State::ReplacePlayerIdWithUnused(::PlayerId id1, ::PlayerId id2)
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

internal::Node& internal::Node::GetChildWithHighestScore()
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

internal::Node& internal::Node::GetRandomChild()
{
	return Children[FMath::RandRange(0, Children.Num() - 1)];
}

internal::MCTSThread::MCTSThread(brFloat maxMoveSearchTimeInSeconds, brFloat maxOpponentTokenSearchTimeInSeconds)
	: m_thread(FRunnableThread::Create(this, TEXT("MCTSThread"), 0, TPri_BelowNormal))
	, m_semaphore(FGenericPlatformProcess::GetSynchEventFromPool(false))
	, m_kill(false)
	, m_pause(true)
	, m_maxMoveSearchTimeInSeconds(maxMoveSearchTimeInSeconds)
	, m_maxOpponentTokenSearchTimeInSeconds(maxOpponentTokenSearchTimeInSeconds)
{
}

internal::MCTSThread::~MCTSThread()
{
	MCTSThread::Stop();
	if(m_thread)
	{
		m_thread->WaitForCompletion();
	}
	
	if(m_semaphore)
	{
		//Cleanup the FEvent
		FGenericPlatformProcess::ReturnSynchEventToPool(m_semaphore);
		m_semaphore = nullptr;
	}

	if (m_thread)
	{
		//Cleanup the worker thread
		delete m_thread;
		m_thread = nullptr;
	}
}

uint32 internal::MCTSThread::Run()
{
	//Initial wait before starting
	FPlatformProcess::Sleep(0.03);

	while (!m_kill)
	{
		if (m_pause)
		{
			//FEvent->Wait(); will "sleep" the thread until it will get a signal "Trigger()"
			m_semaphore->Wait();

			if (m_kill)
			{
				return 0;
			}
		}
		else
		{
			Node root;
			root.State.BoardData = m_request.BoardData;
			root.State.PlayerId = m_request.OpponentId;

			FDateTime const startTime = FDateTime::Now();
			while(!m_kill && (FDateTime::Now() - startTime).GetTotalSeconds() < m_maxMoveSearchTimeInSeconds)
			{
				Node* promisingNode = Select(&root);
				if(promisingNode && promisingNode->State.BoardData.GetStatus() == QuartoBoardData::GameStatus::InProgress)
				{
					Expand(promisingNode, m_request.PlayerId, m_request.OpponentId);
				}
				Node* nodeToExplore = promisingNode;
				if(promisingNode && promisingNode->Children.Num() > 0)
				{
					nodeToExplore = &promisingNode->GetRandomChild();
				}
				PlayerId const winnerId = Simulate(nodeToExplore, m_request.PlayerId, m_request.OpponentId);
				BackPropagate(nodeToExplore, winnerId);
			}

			auto oldEmptySlotCoordinates = m_request.BoardData.GetEmptySlotCoordinates();
			auto oldFreeTokens = m_request.BoardData.GetFreeTokens();
			
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

			m_mutex.Lock();
			{
				//m_request.Result = std::make_tuple(oldFreeTokens[0], oldEmptySlotCoordinates[0]);
			}
			m_mutex.Unlock();
			m_request.IsMoveFound = true;
			m_request.IsTokenFound = true;

			PauseThread();
		}
	}

	return 0;
}

void internal::MCTSThread::Stop()
{
	m_kill = true;
	m_pause = false;
	
	if(m_semaphore)
	{
		//We shall signal "Trigger" the FEvent (in case the Thread is sleeping it shall wake up!!)
		m_semaphore->Trigger();
	}
}

brBool internal::MCTSThread::IsProcessingRequest() const
{
	return static_cast<brBool>(!m_pause);
}

void internal::MCTSThread::RequestNextMoveAndOpponentToken(QuartoTokenData token, QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId)
{
	if(IsProcessingRequest())
	{
		//thread is already looking for a next move
		return;
	}
	
	m_request.BoardData = currentBoard;
	m_request.PlayerId = playerId;
	m_request.OpponentId = opponentId;
	m_request.IsMoveFound = false;
	m_request.IsTokenFound = false;
	m_request.TokenData = token;
	ContinueThread();
}

void internal::MCTSThread::SearchNextMove()
{
	m_request.IsMoveFound = false;

	//do whatever needs to be done
	
	m_request.IsMoveFound = true;
}

void internal::MCTSThread::SearchNextOpponentToken()
{
	m_request.IsTokenFound = false;

	//do whatever needs to be done
	
	m_request.IsTokenFound = true;
}

QuartoTokenData internal::MCTSThread::ConsumeRequestResultToken()
{
	QuartoTokenData result;
	m_mutex.Lock();
	{
		result = m_request.ResultOpponentToken;
	}
	m_mutex.Unlock();
	m_request.IsTokenFound = false;
	return result;
}

QuartoBoardSlotCoordinates internal::MCTSThread::ConsumeRequestResultMove()
{
	QuartoBoardSlotCoordinates result;
	m_mutex.Lock();
	{
		result = m_request.ResultMove;
	}
	m_mutex.Unlock();
	m_request.IsMoveFound = false;
	return result;
}

void internal::MCTSThread::PauseThread()
{
	m_pause = true;
}

void internal::MCTSThread::ContinueThread()
{
	m_pause = false;

	if (m_semaphore)
	{
		//Here is a FEvent signal "Trigger()" -> it will wake up the thread.
		m_semaphore->Trigger();
	}
}

internal::Node* internal::MCTSThread::Select(Node* node)
{
	Node* result = node;
	while (result && result->Children.Num() > 0)
	{
		result = FindBestNodeWithUct(result);
	}
	return result;
}

void internal::MCTSThread::Expand(Node* node, PlayerId playerId, PlayerId opponentId)
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

PlayerId internal::MCTSThread::Simulate(Node* node, PlayerId playerId, PlayerId opponentId)
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

void internal::MCTSThread::BackPropagate(Node* node, PlayerId playerId)
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

internal::Node* internal::MCTSThread::FindBestNodeWithUct(Node* node)
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