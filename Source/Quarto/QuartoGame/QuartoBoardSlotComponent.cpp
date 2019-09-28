// F

#include "QuartoBoardSlotComponent.h"

UQuartoBoardSlotComponent::UQuartoBoardSlotComponent()
: m_xSlot(0)
, m_ySlot(0)
, m_isFree(true)
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