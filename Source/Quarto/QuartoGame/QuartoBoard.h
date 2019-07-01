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
	friend class UQuartoBoardSlotComponent;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Appearance, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Mesh Component"))
	UStaticMeshComponent* m_meshComponent = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category = Appearance, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Slots"))
	TArray<UQuartoBoardSlotComponent*> m_slotComponents;

public:	
	AQuartoBoard();
	void Tick(float DeltaTime) override;

protected:
	void BeginPlay() override;

private:
	void OnBeginSlotCursorOver(UQuartoBoardSlotComponent* slotComponent);
	void OnEndSlotCursorOver(UQuartoBoardSlotComponent* slotComponent);

private:

};
