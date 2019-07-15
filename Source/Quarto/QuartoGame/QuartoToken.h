// F

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuartoToken.generated.h"

class UMaterialInstanceDynamic;
class UQuartoBoardSlotComponent;
class AQuartoGame;

UENUM(BlueprintType)
enum class EQuartoTokenColor : uint8
{
	Color1 		UMETA(DisplayName = "Color1"),
	Color2 		UMETA(DisplayName = "Color2")
};

UENUM(BlueprintType)
enum class EQuartoTokenProperties : uint8
{
	Quadratic 	UMETA(DisplayName = "Quadratic"),
	Round 		UMETA(DisplayName = "Round"),
	Small 		UMETA(DisplayName = "Small"),
	Tall 		UMETA(DisplayName = "Tall"),
	Hole 		UMETA(DisplayName = "Hole")
};

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
	void ShowHighlightForPlayer(bool val);

	void SetIsPlacedOnBoard(bool isPlacedOnBoard);
	bool IsPlacedOnBoard() const { return m_bIsPlacedOnBoard; };
	
	void StartHoverOver(const FVector& location);
	void StopHover();

private:
	AQuartoGame* m_ownerGame;
	UMaterialInstanceDynamic* m_materialInstance;
	FVector m_initialPosition;
	bool m_bIsPlacedOnBoard;
	bool m_bIsHighlightedForPlayer;
	bool m_bIsHovering;
};
