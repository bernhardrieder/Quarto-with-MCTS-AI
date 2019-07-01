// F


#include "QuartoBoard.h"
#include "Components/StaticMeshComponent.h"
#include "QuartoBoardSlotComponent.h"
#include "Misc/Char.h"

// Sets default values
AQuartoBoard::AQuartoBoard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh Component"));
	m_meshComponent->SetNotifyRigidBodyCollision(true);
	m_meshComponent->SetGenerateOverlapEvents(false);
	m_meshComponent->SetSimulatePhysics(false);
	m_meshComponent->SetCollisionProfileName("BlockAllDynamic");
	SetRootComponent(m_meshComponent);

	for(int8 i = 0; i < 16; ++i)
	{
		UQuartoBoardSlotComponent* slotComponent = CreateDefaultSubobject<UQuartoBoardSlotComponent>(FName(*FString::FromInt(i)));
		slotComponent->SetOwnerBoard(this);
		slotComponent->SetupAttachment(RootComponent);
		m_slotComponents.Add(slotComponent);
	}
}

// Called when the game starts or when spawned
void AQuartoBoard::BeginPlay()
{
	Super::BeginPlay();

	TArray<FName> socketNames = m_meshComponent->GetAllSocketNames();
	if(socketNames.Num() == m_slotComponents.Num())
	{
		for (int32 i = 0; i < 16; ++i)
		{
			m_slotComponents[i]->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, socketNames[i]);

			//Slot sockets follow the naming, from X0Y0 to X3Y3
			FString plainString = socketNames[i].GetPlainNameString();
			m_slotComponents[i]->SetX(TChar<TCHAR>::ConvertCharDigitToInt(plainString[1]));
			m_slotComponents[i]->SetY(TChar<TCHAR>::ConvertCharDigitToInt(plainString[3]));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Mismatch of number of sockets for the Quarto Game Board! Needs to be 16 slots."))
	}
}

void AQuartoBoard::OnBeginSlotCursorOver(UQuartoBoardSlotComponent* slotComponent)
{
	UE_LOG(LogTemp, Display, TEXT("slot x%d y%d hovered"), slotComponent->GetX(), slotComponent->GetY());
}

void AQuartoBoard::OnEndSlotCursorOver(UQuartoBoardSlotComponent* slotComponent)
{
	UE_LOG(LogTemp, Display, TEXT("slot x%d y%d hover end"), slotComponent->GetX(), slotComponent->GetY());
}

// Called every frame
void AQuartoBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

