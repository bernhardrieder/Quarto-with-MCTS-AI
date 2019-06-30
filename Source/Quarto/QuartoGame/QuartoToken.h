// F

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuartoToken.generated.h"

class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EQuartoTokenColor : uint8
{
	White 		UMETA(DisplayName = "White"),
	Black 		UMETA(DisplayName = "Black")
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

protected:
	UPROPERTY(VisibleDefaultsOnly, Category = Appearance, BlueprintReadOnly, meta = (AllowPrivateAccess = "true", DisplayName = "Mesh Component"))
	UStaticMeshComponent* m_meshComponent;

	UPROPERTY(EditAnywhere, Category = Properties, BlueprintReadOnly, meta = (DisplayName = "Properties"))
	TArray<EQuartoTokenProperties> m_properties;

public:	
	AQuartoToken();

	void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnLeftMouseButtonClickedOnMesh(UPrimitiveComponent* ClickedComp, FKey ButtonClicked);
	
	UFUNCTION()
	void OnLeftMouseButtonReleasedOnMesh(UPrimitiveComponent* ClickedComp, FKey ButtonReleased);

	void SetColor(EQuartoTokenColor color);

protected:
	void BeginPlay() override;

private:
	EQuartoTokenColor m_color;
	bool m_bIsUsable;
	UMaterialInstanceDynamic* m_materialInstance;
};
