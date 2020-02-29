#include "Quarto/QuartoGame/AI/MonteCarloTreeSearch.h"

#include "HAL/PlatformProcess.h"
#include "HAL/RunnableThread.h"
#include "HAL/Event.h"

using namespace ai::mcts;

MonteCarloTreeSearch::MonteCarloTreeSearch(brFloat maxMoveSearchTimeInSeconds, brFloat maxOpponentTokenSearchTimeInSeconds)
{
	m_threadWorker = new internal::MCTSThread(maxMoveSearchTimeInSeconds, maxOpponentTokenSearchTimeInSeconds);
}

MonteCarloTreeSearch::~MonteCarloTreeSearch()
{
	delete m_threadWorker;
}

void MonteCarloTreeSearch::FindNextOpponentToken(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId) const
{
	m_threadWorker->RequestNextOpponentToken(currentBoard, playerId, opponentId);
}

void MonteCarloTreeSearch::FindNextMove(QuartoTokenData const& token, QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId) const
{
	m_threadWorker->RequestNextMove(token, currentBoard, playerId, opponentId);
}

brBool MonteCarloTreeSearch::IsLookingForNextMove() const
{
	return m_threadWorker->IsProcessingAnyRequest();
}

brBool MonteCarloTreeSearch::IsLookingForNextOpponentToken() const
{
	return m_threadWorker->IsProcessingAnyRequest();
}

brBool MonteCarloTreeSearch::HasFoundNextMove() const
{
	return m_threadWorker->IsMoveRequestFinished();
}

brBool MonteCarloTreeSearch::HasFoundNextOpponentToken() const
{
	return m_threadWorker->IsOpponentTokenRequestFinished();
}

QuartoBoardSlotCoordinates MonteCarloTreeSearch::GetNextMoveCoordinates() const
{
	return m_threadWorker->ConsumeRequestResultNextMove();
}

QuartoTokenData MonteCarloTreeSearch::GetNextOpponentToken() const
{
	return m_threadWorker->ConsumeRequestResultOpponentToken();
}


TArray<internal::State> internal::State::GetAllPossibleStates(QuartoTokenData const* token) const
{
	TArray<State> states;
	auto addState = [&](QuartoBoardSlotCoordinates const& c, QuartoTokenData const& t)
	{
		State newState;
		newState.BoardData = BoardData;
		newState.BoardData.SetTokenOnBoard(c, t);
		states.Add(newState);
	};
	
	for(auto const& slotCoordinates : BoardData.GetEmptySlotCoordinates())
	{
		if(!token)
		{
			for (auto const& freeToken : BoardData.GetFreeTokens())
			{
				addState(slotCoordinates, freeToken);
			}
		}
		else
		{
			addState(slotCoordinates, *token);
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
	m_moveRequest.IsProcessed = true;
	m_opponentTokenRequest.IsProcessed = true;
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
			if(!m_opponentTokenRequest.IsProcessed)
			{
				SearchNextOpponentToken();
			}
			
			if(!m_moveRequest.IsProcessed)
			{
				SearchNextMove();
			}

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

brBool internal::MCTSThread::IsProcessingAnyRequest() const
{
	return static_cast<brBool>(!m_pause);
}

void internal::MCTSThread::RequestNextMove(QuartoTokenData const& token, QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId)
{
	if(IsProcessingAnyRequest())
	{
		return;
	}
	
	m_moveRequest.BoardData = currentBoard;
	m_moveRequest.TokenData = token;
	m_moveRequest.PlayerId = playerId;
	m_moveRequest.OpponentId = opponentId;
	m_moveRequest.IsMoveFound = false;
	m_moveRequest.IsProcessed = false;
	ContinueThread();
}

void internal::MCTSThread::RequestNextOpponentToken(QuartoBoardData const& currentBoard, PlayerId playerId, PlayerId opponentId)
{
	if (IsProcessingAnyRequest())
	{
		return;
	}

	m_opponentTokenRequest.BoardData = currentBoard;
	m_opponentTokenRequest.PlayerId = playerId;
	m_opponentTokenRequest.OpponentId = opponentId;
	m_opponentTokenRequest.IsTokenFound = false;
	m_opponentTokenRequest.IsProcessed = false;
	ContinueThread();
}

void internal::MCTSThread::SearchNextMove()
{
	m_moveRequest.IsMoveFound = false;

	auto const winnerBoard = 
		SearchNextDraw(
			m_maxMoveSearchTimeInSeconds, 
			&m_moveRequest.BoardData, 
			&m_moveRequest.TokenData, 
			m_moveRequest.PlayerId, 
			m_moveRequest.OpponentId, 
			false);
	
	auto oldEmptySlotCoordinates = m_moveRequest.BoardData.GetEmptySlotCoordinates();
	for (auto& slotCoordinates : winnerBoard.GetEmptySlotCoordinates())
	{
		oldEmptySlotCoordinates.Remove(slotCoordinates);
	}

	m_mutex.Lock();
	{
		if (oldEmptySlotCoordinates.Num() == 1)
		{
			m_moveRequest.ResultMove = oldEmptySlotCoordinates[0];
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: No slotcoordinates were found with the Monte Carlo Tree Search! Random free slot coordinate will be used."));
			auto const& freeCoords = m_opponentTokenRequest.BoardData.GetEmptySlotCoordinates();
			m_moveRequest.ResultMove = freeCoords[FMath::RandRange(0, freeCoords.Num() - 1)];
		}
	}
	m_mutex.Unlock();
	
	m_moveRequest.IsMoveFound = true;
	m_moveRequest.IsProcessed = true;
}

void internal::MCTSThread::SearchNextOpponentToken()
{
	m_opponentTokenRequest.IsTokenFound = false;

	auto const winnerBoard = 
		SearchNextDraw(
			m_maxOpponentTokenSearchTimeInSeconds, 
			&m_opponentTokenRequest.BoardData, 
			nullptr,
			m_opponentTokenRequest.PlayerId,
			m_opponentTokenRequest.OpponentId,
			true);
	
	auto oldFreeTokens = m_opponentTokenRequest.BoardData.GetFreeTokens();
	for (auto& token : winnerBoard.GetFreeTokens())
	{
		oldFreeTokens.Remove(token);
	}

	m_mutex.Lock();
	{
		if (oldFreeTokens.Num() == 1)
		{
			m_opponentTokenRequest.ResultToken = oldFreeTokens[0];
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: No token was found with the Monte Carlo Tree Search! Random free token will be used."));
			auto const& freeTokens = m_opponentTokenRequest.BoardData.GetFreeTokens();
			m_opponentTokenRequest.ResultToken = freeTokens[FMath::RandRange(0, freeTokens.Num() - 1)];
		}
	}
	m_mutex.Unlock();

	m_opponentTokenRequest.IsTokenFound = true;
	m_opponentTokenRequest.IsProcessed = true;
}

QuartoTokenData internal::MCTSThread::ConsumeRequestResultOpponentToken()
{
	QuartoTokenData result;
	m_mutex.Lock();
	{
		result = m_opponentTokenRequest.ResultToken;
	}
	m_mutex.Unlock();
	m_opponentTokenRequest.IsTokenFound = false;
	return result;
}

QuartoBoardSlotCoordinates internal::MCTSThread::ConsumeRequestResultNextMove()
{
	QuartoBoardSlotCoordinates result;
	m_mutex.Lock();
	{
		result = m_moveRequest.ResultMove;
	}
	m_mutex.Unlock();
	m_moveRequest.IsMoveFound = false;
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

QuartoBoardData internal::MCTSThread::SearchNextDraw(brFloat maxSearchTime, QuartoBoardData const* boardData, QuartoTokenData const* tokenData, PlayerId playerId, PlayerId opponentId, brBool negate) const
{
	Node root;
	root.State.BoardData = *boardData;
	root.State.PlayerId = opponentId;

	FDateTime const startTime = FDateTime::Now();
	while (!m_kill && (FDateTime::Now() - startTime).GetTotalSeconds() < maxSearchTime)
	{
		Node* promisingNode = Select(&root);
		if (promisingNode && promisingNode->State.BoardData.GetStatus() == QuartoBoardData::GameStatus::InProgress)
		{
			Expand(promisingNode, playerId, opponentId, tokenData);
		}
		Node* nodeToExplore = promisingNode;
		if (promisingNode && promisingNode->Children.Num() > 0)
		{
			nodeToExplore = &promisingNode->GetRandomChild();
		}
		PlayerId const winnerId = Simulate(nodeToExplore, playerId, opponentId, negate);
		BackPropagate(nodeToExplore, winnerId, negate);
	}
	
	return root.GetChildWithHighestScore().State.BoardData;
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

void internal::MCTSThread::Expand(Node* node, PlayerId playerId, PlayerId opponentId, QuartoTokenData const* token)
{
	if(!node)
	{
		return;
	}

	TArray<State> possibleStates = node->State.GetAllPossibleStates(token);
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

PlayerId internal::MCTSThread::Simulate(Node* node, PlayerId playerId, PlayerId opponentId, brBool negate)
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
			node->Parent->State.WinScore = negate ? INT_MAX : INT_MIN;
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

void internal::MCTSThread::BackPropagate(Node* node, PlayerId playerId, brBool negate)
{
	Node* tmpNode = node;
	while(tmpNode)
	{
		State& state = tmpNode->State;
		++(state.VisitCount);
		if((!negate && state.PlayerId == playerId) 
			|| (negate && state.PlayerId != playerId))
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

	Node* bestNode = nullptr;
	brU32 const parentVisit = node->State.VisitCount;
	brFloat highestUctValue = brFloatMin;
	for(Node& childNode : node->Children)
	{
		brFloat const uctValue = uctValueFct(parentVisit, childNode.State.WinScore, childNode.State.VisitCount);
		if(uctValue > highestUctValue || !bestNode)
		{
			bestNode = &childNode;
			highestUctValue = uctValue;
		}
	}
	
	return bestNode;
}