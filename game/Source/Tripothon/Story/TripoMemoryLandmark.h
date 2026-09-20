#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoMemoryLandmark.generated.h"
class UStaticMeshComponent;
class UTextRenderComponent;
class UTripoIdentityComponent;
UENUM(BlueprintType)
enum class ETripoMemoryKind : uint8 { SharedBase, SharedRoute, MessageAndPhoto };
// Presentation-only actor: no Pawn collision or interactor, even when projection is enabled.
UCLASS()
class TRIPOTHON_API ATripoMemoryLandmark : public AActor
{
    GENERATED_BODY()
public:
    ATripoMemoryLandmark();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float Delta) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Visual;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTripoIdentityComponent> Identity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ETripoMemoryKind Kind = ETripoMemoryKind::SharedBase;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Reference = TEXT("Original map reference pending");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bProjectionVisible = true;
};
