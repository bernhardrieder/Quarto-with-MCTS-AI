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
	Color1 = 0x01 	UMETA(DisplayName = "Color1"),
	Color2 = 0x02	UMETA(DisplayName = "Color2")
};

UENUM(BlueprintType)
enum class EQuartoTokenProperties : uint8
{
	Quadratic	= 0x01	UMETA(DisplayName = "Quadratic"),
	Round		= 0x02 	UMETA(DisplayName = "Round"),
	Small		= 0x04	UMETA(DisplayName = "Small"),
	Tall		= 0x08	UMETA(DisplayName = "Tall"),
	Hole		= 0x10	UMETA(DisplayName = "Hole")
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

	TArray<EQuartoTokenProperties> GetProperties() const { return m_properties; }
	bool HasAtLeastOneMatchingProperty(AQuartoToken* other) const;
	int32 GetPropertiesArrayAsBitMask() const { return m_propertiesArrayAsBitmask; }

	EQuartoTokenColor GetColor() const { return m_color; }
	int32 GetColorAsBitMask() const { return static_cast<int32>(m_color); }

private:
	AQuartoGame* m_ownerGame;
	UMaterialInstanceDynamic* m_materialInstance;
	FVector m_initialPosition;
	int32 m_propertiesArrayAsBitmask;
	bool m_bIsPlacedOnBoard;
	bool m_bIsHighlightedForPlayer;
	bool m_bIsHovering;
};
