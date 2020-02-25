// F

#pragma once

#include "Common/UnrealCommon.h"
#include "GameFramework/Actor.h"
#include "QuartoCommon.h"
#include "QuartoData.h"
#include "QuartoToken.generated.h"

class UMaterialInstanceDynamic;
class UQuartoBoardSlotComponent;
class AQuartoGame;

UCLASS()
class QUARTO_API AQuartoToken : public AActor
{
	GENERATED_BODY()

	friend class AQuartoGame;

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Appearance, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Mesh Component"))
	UStaticMeshComponent* m_meshComponent;

	UPROPERTY(EditDefaultsOnly, Category = Properties, BlueprintReadOnly, meta = (DisplayName = "Properties"))
	TArray<EQuartoTokenProperties> m_properties;

	UPROPERTY(EditDefaultsOnly, Category = Properties, BlueprintReadOnly, meta = (DisplayName = "Color"))
	EQuartoTokenColor m_color;

public:	
	AQuartoToken();
	void Reset() override;

protected:
	void BeginPlay() override;

public:
	void ShowHighlightForPlayer(brBool val);

	void SetIsPlacedOnBoard(brBool isPlacedOnBoard);
	brBool IsPlacedOnBoard() const { return m_bIsPlacedOnBoard; };
	
	void StartHoverOver(const FVector& location);
	void StopHover();

	QuartoTokenData const& GetData() const { return m_data; }
	
private:
	AQuartoGame* m_ownerGame;
	UMaterialInstanceDynamic* m_materialInstance;
	FVector m_initialPosition;
	QuartoTokenData m_data;
	brBool m_bIsPlacedOnBoard;
	brBool m_bIsHighlightedForPlayer;
	brBool m_bIsHovering;
};
