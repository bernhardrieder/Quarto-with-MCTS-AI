// F


#include "QuartoBoard.h"
#include "Components/StaticMeshComponent.h"
#include "QuartoBoardSlotComponent.h"
#include "Misc/Char.h"
#include "DrawDebugHelpers.h"

AQuartoBoard::AQuartoBoard()
{
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

void AQuartoBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UQuartoBoardSlotComponent* AQuartoBoard::TraceForSlot(const FVector& Start, const FVector& End, bool bDrawDebugHelpers) const
{
	FHitResult hitResult;
	GetWorld()->LineTraceSingleByChannel(hitResult, Start, End, ECC_Visibility);

	return FindSlot(std::move(hitResult), bDrawDebugHelpers);
}

UQuartoBoardSlotComponent* AQuartoBoard::FindSlot(const FHitResult& hitResult, bool bDrawDebugHelpers) const
{
	UQuartoBoardSlotComponent* slot = nullptr;
	if (hitResult.IsValidBlockingHit() && hitResult.Component.Get())
	{
		slot = Cast<UQuartoBoardSlotComponent>(hitResult.Component.Get());

		if (slot && bDrawDebugHelpers)
		{
			DrawDebugSphere(GetWorld(), slot->GetComponentLocation(), 100.f, 6, FColor::Red);
		}
	}
	return slot;
}
