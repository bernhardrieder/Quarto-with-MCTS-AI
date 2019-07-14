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

	FORCEINLINE void SetX(int32 xSlot) { m_xSlot = xSlot; }
	FORCEINLINE int32 GetX() const { return m_xSlot; }
	FORCEINLINE void SetY(int32 ySlot) { m_ySlot = ySlot; }
	FORCEINLINE int32 GetY() const { return m_ySlot; }

private:
	int32 m_xSlot;
	int32 m_ySlot;
};
