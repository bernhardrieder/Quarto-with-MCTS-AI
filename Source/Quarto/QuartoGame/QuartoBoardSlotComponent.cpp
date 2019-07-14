// F

#include "QuartoBoardSlotComponent.h"
#include "QuartoBoard.h"
#include "QuartoToken.h"

UQuartoBoardSlotComponent::UQuartoBoardSlotComponent()
: m_xSlot(0)
, m_ySlot(0)
, m_placedToken(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;

	CapsuleHalfHeight = 150.f;
	CapsuleRadius = 150.f;

	UPrimitiveComponent::SetSimulatePhysics(false);
	UPrimitiveComponent::SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	USceneComponent::SetMobility(EComponentMobility::Stationary);

#if UE_EDITOR
	bHiddenInGame = false;
#endif
}

void UQuartoBoardSlotComponent::PlaceToken(AQuartoToken* token)
{
	m_placedToken = token;
	m_placedToken->SetActorLocation(this->GetComponentLocation());
}

void UQuartoBoardSlotComponent::ReleasePlacedToken()
{
	m_placedToken->RemoveFromBoard();
	m_placedToken = nullptr;
}

void UQuartoBoardSlotComponent::HoverToken(AQuartoToken* token)
{
	token->SetActorLocation(this->GetComponentLocation() + FVector::UpVector * 100.f);
}
