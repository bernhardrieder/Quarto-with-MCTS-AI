// F

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "QuartoBoardSlotComponent.generated.h"

class AQuartoBoard;

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class QUARTO_API UQuartoBoardSlotComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:	
	UQuartoBoardSlotComponent();
	void BeginPlay() override;

	FORCEINLINE void SetOwnerBoard(AQuartoBoard* board) { m_ownerBoard = board; }
	FORCEINLINE void SetX(int32 xSlot) { m_xSlot = xSlot; }
	FORCEINLINE int32 GetX() const { return m_xSlot; }
	FORCEINLINE void SetY(int32 ySlot) { m_ySlot = ySlot; }
	FORCEINLINE int32 GetY() const { return m_ySlot; }

protected:
	UFUNCTION()
	void OnBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnEndCursorOver(UPrimitiveComponent* TouchedComponent);

private:
	int32 m_xSlot;
	int32 m_ySlot;
	AQuartoBoard* m_ownerBoard;
};
