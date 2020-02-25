// F


#include "QuartoBoard.h"
#include "Components/StaticMeshComponent.h"
#include "QuartoBoardSlotComponent.h"
#include "DrawDebugHelpers.h"
#include "QuartoToken.h"

AQuartoBoard::AQuartoBoard()
: m_lastFoundFreeSlot(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	m_meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh Component"));
	m_meshComponent->SetNotifyRigidBodyCollision(true);
	m_meshComponent->SetGenerateOverlapEvents(false);
	m_meshComponent->SetSimulatePhysics(false);
	m_meshComponent->SetCollisionProfileName("BlockAllDynamic");
	SetRootComponent(m_meshComponent);

	for(brU8 i = 0; i < QUARTO_BOARD_AVAILABLE_SLOTS; ++i)
	{
		UQuartoBoardSlotComponent* slotComponent = CreateDefaultSubobject<UQuartoBoardSlotComponent>(FName(*FString::FromInt(i)));
		slotComponent->SetupAttachment(RootComponent);
		m_slotComponents.Add(slotComponent);
	}
}

void AQuartoBoard::BeginPlay()
{
	Super::BeginPlay();

	TArray<FName> socketNames = m_meshComponent->GetAllSocketNames();
	if(socketNames.Num() == m_slotComponents.Num())
	{
		for (brU32 i = 0; i < QUARTO_BOARD_AVAILABLE_SLOTS; ++i)
		{
			m_slotComponents[i]->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, socketNames[i]);

			//Slot sockets follow the naming, from X0Y0 to X3Y3
			FString plainString = socketNames[i].GetPlainNameString();
			brU32 x = TChar<TCHAR>::ConvertCharDigitToInt(plainString[1]);
			brU32 y = TChar<TCHAR>::ConvertCharDigitToInt(plainString[3]);
			m_slotComponents[i]->SetCoordinates({ x, y });
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Mismatch of number of sockets for the Quarto Game Board! Needs to be 16 slots."))
	}
}

UQuartoBoardSlotComponent* AQuartoBoard::FindSlotComponent(QuartoBoardSlotCoordinates const& coordinates)
{
	return *(m_slotComponents.FindByPredicate([&coordinates](UQuartoBoardSlotComponent* comp) {return comp && comp->GetCoordinates() == coordinates; }));
}

void AQuartoBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AQuartoBoard::Reset()
{
	for(UQuartoBoardSlotComponent* slot : m_slotComponents)
	{
		if(slot)
		{
			slot->SetIsFree(true);
		}
	}
	m_data.Reset();
	m_lastFoundFreeSlot = nullptr;
	Super::Reset();
}

bool AQuartoBoard::CanFindFreeSlot(const FHitResult& hitResult, brBool bDrawDebugHelpers)
{
	UQuartoBoardSlotComponent* slot = nullptr;
	if (hitResult.IsValidBlockingHit() && hitResult.Component.Get())
	{	
		slot = Cast<UQuartoBoardSlotComponent>(hitResult.Component.Get());

		if (bDrawDebugHelpers)
		{
			DrawDebugSphere(GetWorld(), hitResult.ImpactPoint, 100.f, 6, FColor::Orange);

			if(slot)
			{
				DrawDebugSphere(GetWorld(), slot->GetComponentLocation(), 100.f, 6, FColor::Red);
			}
		}
	}

	if (slot && slot->IsFree())
	{
		m_lastFoundFreeSlot = slot;
		return true;
	}

	return false;
}

void AQuartoBoard::HoverTokenOverLastFoundFreeSlot(AQuartoToken* token)
{
	HoverTokenOverSlot(token, m_lastFoundFreeSlot->GetCoordinates());
}

void AQuartoBoard::HoverTokenOverSlot(AQuartoToken* token, QuartoBoardSlotCoordinates const& slotCoordinates)
{
	UQuartoBoardSlotComponent* slotComponent = FindSlotComponent(slotCoordinates);
	if (!slotComponent || !token)
	{
		return;
	}

	token->StartHoverOver(slotComponent->GetComponentLocation());
}

void AQuartoBoard::PlaceTokenOnLastFoundFreeSlot(AQuartoToken* token)
{
	PlaceTokenOnBoardSlot(token, m_lastFoundFreeSlot->GetCoordinates());
}

void AQuartoBoard::PlaceTokenOnBoardSlot(AQuartoToken* token, QuartoBoardSlotCoordinates const& slotCoordinates)
{
	UQuartoBoardSlotComponent* slotComponent = FindSlotComponent(slotCoordinates);
	if (!token || !slotComponent || !slotComponent->IsFree())
	{
		return;
	}
	
	token->SetIsPlacedOnBoard(true);
	token->SetActorLocation(slotComponent->GetComponentLocation());
	slotComponent->SetIsFree(false);
	m_data.SetTokenOnBoard(slotCoordinates, token->GetData());
}