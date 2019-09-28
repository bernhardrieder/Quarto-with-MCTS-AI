// F


#include "QuartoToken.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "QuartoBoardSlotComponent.h"


AQuartoToken::AQuartoToken()
: m_meshComponent(nullptr)
, m_materialInstance(nullptr)
, m_initialPosition(FVector::ZeroVector)
, m_bIsPlacedOnBoard(false)
, m_bIsHighlightedForPlayer(false)
, m_bIsHovering(false)
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
	m_initialPosition = GetActorLocation();
	m_data = QuartoTokenData(m_color, m_properties);
}

void AQuartoToken::Reset()
{
	SetIsPlacedOnBoard(false);
	StopHover();
	SetActorLocation(m_initialPosition);
	Super::Reset();
}

void AQuartoToken::ShowHighlightForPlayer(brBool val)
{
	if (!m_materialInstance || m_bIsHighlightedForPlayer == val)
	{
		return;
	}

	m_bIsHighlightedForPlayer = val;
	m_materialInstance->SetScalarParameterValue("HighlightStrength", val ? 1.f : 0.f);
}

void AQuartoToken::SetIsPlacedOnBoard(brBool isPlacedOnBoard)
{
	m_bIsPlacedOnBoard = isPlacedOnBoard;
	ShowHighlightForPlayer(false);
}

void AQuartoToken::StartHoverOver(const FVector& location)
{
	SetActorLocation(location + FVector::UpVector * 100.f);
	m_bIsHovering = true;
}

void AQuartoToken::StopHover()
{
	if(m_bIsHovering)
	{
		SetActorLocation(GetActorLocation() - FVector::UpVector * 100.f);
		m_bIsHovering = false;
	}
}