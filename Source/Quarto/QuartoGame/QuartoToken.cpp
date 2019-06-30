// F


#include "QuartoToken.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
AQuartoToken::AQuartoToken()
: m_meshComponent(nullptr)
, m_bIsUsable(true)
, m_materialInstance(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh Component"));
	m_meshComponent->SetNotifyRigidBodyCollision(true);
	m_meshComponent->SetGenerateOverlapEvents(false);
	m_meshComponent->SetSimulatePhysics(false);
	m_meshComponent->SetCollisionProfileName("BlockAllDynamic");
	m_meshComponent->OnClicked.AddDynamic(this, &AQuartoToken::OnLeftMouseButtonClickedOnMesh);
	m_meshComponent->OnReleased.AddDynamic(this, &AQuartoToken::OnLeftMouseButtonReleasedOnMesh);

	m_materialInstance = UMaterialInstanceDynamic::Create(m_meshComponent->GetMaterial(0), m_meshComponent);
	m_meshComponent->SetMaterial(0, m_materialInstance);

	SetRootComponent(m_meshComponent);
}

void AQuartoToken::OnLeftMouseButtonClickedOnMesh(UPrimitiveComponent* ClickedComp, FKey ButtonClicked)
{
}

void AQuartoToken::OnLeftMouseButtonReleasedOnMesh(UPrimitiveComponent* ClickedComp, FKey ButtonReleased)
{
}

void AQuartoToken::SetColor(EQuartoTokenColor color)
{
	m_color = color;
	
	if (!m_materialInstance)
	{
		return;
	}

	FLinearColor linearColor;
	switch(color)
	{
	case EQuartoTokenColor::White:
		linearColor = FLinearColor::White;
		break;
	case EQuartoTokenColor::Black:
		linearColor = FLinearColor::Black;
	}
	
	m_materialInstance->SetVectorParameterValue("DiffuseColor", linearColor);
}

void AQuartoToken::BeginPlay()
{
	Super::BeginPlay();
}

void AQuartoToken::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

