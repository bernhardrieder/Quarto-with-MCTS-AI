// F


#include "QuartoToken.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AQuartoToken::AQuartoToken()
: m_meshComponent(nullptr)
, m_bIsUsable(true)
, m_materialInstance(nullptr)
, m_isHighlightedForPlayer(false)
{
	PrimaryActorTick.bCanEverTick = false;

	m_meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh Component"));
	m_meshComponent->SetNotifyRigidBodyCollision(true);
	m_meshComponent->SetGenerateOverlapEvents(false);
	m_meshComponent->SetSimulatePhysics(false);
	m_meshComponent->SetCollisionProfileName("BlockAllDynamic");

	SetRootComponent(m_meshComponent);
}

void AQuartoToken::BeginPlay()
{
	Super::BeginPlay();

	m_materialInstance = UMaterialInstanceDynamic::Create(m_meshComponent->GetMaterial(0), m_meshComponent);
	m_meshComponent->SetMaterial(0, m_materialInstance);
}

void AQuartoToken::ShowHighlightForPlayer(bool val)
{
	if (!m_materialInstance || m_isHighlightedForPlayer == val)
	{
		return;
	}

	m_isHighlightedForPlayer = val;
	m_materialInstance->SetScalarParameterValue("HighlightStrength", val ? 1.f : 0.f);
}
