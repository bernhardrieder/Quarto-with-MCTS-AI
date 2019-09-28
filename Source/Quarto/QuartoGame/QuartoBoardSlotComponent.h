// F

#pragma once

#include "Common/UnrealCommon.h"
#include "Components/CapsuleComponent.h"
#include "QuartoBoardSlotComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class QUARTO_API UQuartoBoardSlotComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:	
	UQuartoBoardSlotComponent();

	FORCEINLINE void SetX(int32 xSlot) { m_xSlot = xSlot; }
	FORCEINLINE brS32 GetX() const { return m_xSlot; }
	FORCEINLINE void SetY(int32 ySlot) { m_ySlot = ySlot; }
	FORCEINLINE brS32 GetY() const { return m_ySlot; }

	void SetIsFree(bool isFree) { m_isFree = isFree; }
	brBool IsFree() const { return m_isFree; }

private:
	brS32 m_xSlot;
	brS32 m_ySlot;
	brBool m_isFree;
};