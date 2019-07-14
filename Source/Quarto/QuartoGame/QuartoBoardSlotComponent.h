// F

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "QuartoBoardSlotComponent.generated.h"

class AQuartoBoard;
class AQuartoToken;

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

	void PlaceToken(AQuartoToken* token);
	AQuartoToken* GetPlacedToken() const { return m_placedToken; }
	bool HasPlacedToken() const { return !!m_placedToken; }
	void ReleasePlacedToken();
	void HoverToken(AQuartoToken* token);

private:
	int32 m_xSlot;
	int32 m_ySlot;
	AQuartoToken* m_placedToken;
};
