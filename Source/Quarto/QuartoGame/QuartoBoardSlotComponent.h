// F

#pragma once

#include "Components/CapsuleComponent.h"
#include "Quarto/Common/UnrealCommon.h"
#include "Quarto/QuartoGame/QuartoBoard.h"
#include "QuartoBoardSlotComponent.generated.h"

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class QUARTO_API UQuartoBoardSlotComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:	
	UQuartoBoardSlotComponent();

	FORCEINLINE void SetCoordinates(QuartoBoardSlotCoordinates const& coordinates) { m_coordinates = coordinates; }
	FORCEINLINE QuartoBoardSlotCoordinates GetCoordinates() const { return m_coordinates; }

	void SetIsFree(bool isFree) { m_isFree = isFree; }
	brBool IsFree() const { return m_isFree; }

private:
	QuartoBoardSlotCoordinates m_coordinates;
	brBool m_isFree;
};