// F

#include "QuartoBoardSlotComponent.h"
#include "QuartoBoard.h"

UQuartoBoardSlotComponent::UQuartoBoardSlotComponent()
: m_xSlot(0)
, m_ySlot(0)
, m_ownerBoard(nullptr)

{
	PrimaryComponentTick.bCanEverTick = true;

	CapsuleHalfHeight = 150.f;
	CapsuleRadius = 150.f;

	//UPrimitiveComponent::SetNotifyRigidBodyCollision(false);
	UPrimitiveComponent::SetSimulatePhysics(false);
	UPrimitiveComponent::SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	USceneComponent::SetMobility(EComponentMobility::Stationary);
	UPrimitiveComponent::OnBeginCursorOver.AddDynamic(this, &UQuartoBoardSlotComponent::OnBeginCursorOver);
	UPrimitiveComponent::OnEndCursorOver.AddDynamic(this, &UQuartoBoardSlotComponent::OnEndCursorOver);

#if UE_EDITOR
	bHiddenInGame = false;
#endif
}

void UQuartoBoardSlotComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UQuartoBoardSlotComponent::OnBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if(m_ownerBoard)
	{
		m_ownerBoard->OnBeginSlotCursorOver(this);
	}
}

void UQuartoBoardSlotComponent::OnEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	if(m_ownerBoard)
	{
		m_ownerBoard->OnEndSlotCursorOver(this);
	}
}
