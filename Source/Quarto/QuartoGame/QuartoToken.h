// F

#pragma once

#include "Common/UnrealCommon.h"
#include "GameFramework/Actor.h"
#include "QuartoToken.generated.h"

class UMaterialInstanceDynamic;
class UQuartoBoardSlotComponent;
class AQuartoGame;


UENUM(BlueprintType)
enum class EQuartoTokenColor : uint8 /*brU8 -> UE header tool doesn't like it*/
{
	Color1 = 0x01 	UMETA(DisplayName = "Color1"),
	Color2 = 0x02	UMETA(DisplayName = "Color2")
};

UENUM(BlueprintType)
enum class EQuartoTokenProperties : uint8 /*brU8 -> UE header tool doesn't like it*/
{
	Quadratic	= 0x01	UMETA(DisplayName = "Quadratic"),
	Round		= 0x02 	UMETA(DisplayName = "Round"),
	Small		= 0x04	UMETA(DisplayName = "Small"),
	Tall		= 0x08	UMETA(DisplayName = "Tall"),
	Hole		= 0x10	UMETA(DisplayName = "Hole"),
	Filled		= 0x20	UMETA(DisplayName = "Filled")
};

struct QuartoTokenData
{
	QuartoTokenData()
		: QuartoTokenData(EQuartoTokenColor::Color1, {}) {};
	QuartoTokenData(EQuartoTokenColor color, EQuartoTokenProperties property1, EQuartoTokenProperties property2, EQuartoTokenProperties property3)
		: QuartoTokenData(color, { property1, property2, property3 }) {}
	QuartoTokenData(EQuartoTokenColor color, TArray<EQuartoTokenProperties> properties);

	TArray<EQuartoTokenProperties> GetProperties() const { return m_properties; }
	bool HasAtLeastOneMatchingProperty(QuartoTokenData& other) const { return (m_propertiesBitmask & other.m_propertiesBitmask) > 0; }
	brU32 GetPropertiesBitMask() const { return m_propertiesBitmask; }

	EQuartoTokenColor GetColor() const { return m_color; }
	brU32 GetColorBitMask() const { return static_cast<brU32>(m_color); }

	brBool IsValid() const { return m_propertiesBitmask > 0; }
	void Invalidate() { m_propertiesBitmask = 0; }
	
public:
	static TArray<QuartoTokenData> PossiblePermutations;
	
private:
	EQuartoTokenColor m_color;
	TArray<EQuartoTokenProperties> m_properties;
	brU32 m_propertiesBitmask;
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
