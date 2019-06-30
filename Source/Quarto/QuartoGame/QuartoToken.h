// F

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuartoToken.generated.h"

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
	UStaticMeshComponent* m_meshComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = Properties, BlueprintReadOnly, meta = (DisplayName = "Color"))
	EQuartoTokenColor m_color;
	
	UPROPERTY(EditAnywhere, Category = Properties, BlueprintReadOnly, meta = (DisplayName = "Properties"))
	TArray<EQuartoTokenProperties> m_properties;

public:	
	// Sets default values for this actor's properties
	AQuartoToken();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
