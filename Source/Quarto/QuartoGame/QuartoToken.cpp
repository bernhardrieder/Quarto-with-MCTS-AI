// F


#include "QuartoToken.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AQuartoToken::AQuartoToken()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh Component"));
	m_meshComponent->SetNotifyRigidBodyCollision(true);
	m_meshComponent->SetGenerateOverlapEvents(false);
	m_meshComponent->SetSimulatePhysics(false);
	m_meshComponent->SetCollisionProfileName("BlockAllDynamic");
	SetRootComponent(m_meshComponent);
}

// Called when the game starts or when spawned
void AQuartoToken::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQuartoToken::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

