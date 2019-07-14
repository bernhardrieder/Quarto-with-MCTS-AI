// F

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuartoBoard.generated.h"

class UQuartoBoardSlotComponent;

UCLASS()
class QUARTO_API AQuartoBoard : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Appearance, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Mesh Component"))
	UStaticMeshComponent* m_meshComponent = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category = Appearance, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Slots"))
	TArray<UQuartoBoardSlotComponent*> m_slotComponents;

public:	
	AQuartoBoard();
	void Tick(float DeltaTime) override;

	UQuartoBoardSlotComponent* TraceForSlot(const FVector& Start, const FVector& End, bool bDrawDebugHelpers) const;
	UQuartoBoardSlotComponent* FindSlot(const FHitResult& hitResult, bool bDrawDebugHelpers) const;

protected:
	void BeginPlay() override;

};
