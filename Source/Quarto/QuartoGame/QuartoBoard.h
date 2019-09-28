// F

#pragma once

#include "Common/UnrealCommon.h"
#include "GameFramework/Actor.h"
#include "QuartoToken.h"
#include "QuartoBoard.generated.h"

class AQuartoToken;
class UQuartoBoardSlotComponent;
class AQuartoGame;

struct QuartoBoardData
{
	QuartoTokenData m_tokensOnBoardGrid[16]; //xDim, yDim = 4
};

UCLASS()
class QUARTO_API AQuartoBoard : public AActor
{
	GENERATED_BODY()

	friend class AQuartoGame;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Appearance, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Mesh Component"))
	UStaticMeshComponent* m_meshComponent = nullptr;

	UPROPERTY(VisibleDefaultsOnly, Category = Appearance, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Slots"))
	TArray<UQuartoBoardSlotComponent*> m_slotComponents;

public:	
	AQuartoBoard();
	void Tick(float DeltaTime) override;
	void Reset() override;

	bool CanFindFreeSlot(const FHitResult& hitResult, brBool bDrawDebugHelpers);
	UQuartoBoardSlotComponent* GetLastFoundFreeSlot() const { return m_lastFoundFreeSlot; }
	void HoverTokenOverLastFoundFreeSlot(AQuartoToken* token);
	void PlaceTokenOnLastFoundFreeSlot(AQuartoToken* token);

	brS32 GetNumberOfFreeSlots() const;
	QuartoBoardData const& GetData() const { return m_data; }

protected:
	void BeginPlay() override;

private:
	AQuartoGame* m_ownerGame;
	UQuartoBoardSlotComponent* m_lastFoundFreeSlot;
	QuartoBoardData m_data;
};
