#pragma once
#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "TripoInteractionTarget.generated.h"
class ATripoCharacter;
class UMeshComponent;
class UMaterialInterface;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTripoFocusedInteraction, ATripoCharacter*, Player);
/** A single-player, aim-selected interaction surface. Attach to an object's visible control. */
UCLASS(ClassGroup=(Tripo), meta=(BlueprintSpawnableComponent))
class TRIPOTHON_API UTripoInteractionTarget : public UBoxComponent
{
    GENERATED_BODY()
public:
    UTripoInteractionTarget();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") FText Prompt;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") float Reach = 240;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") TObjectPtr<UMeshComponent> HighlightMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") TObjectPtr<UMaterialInterface> HighlightMaterial;
    UPROPERTY(BlueprintAssignable, Category="Interaction") FTripoFocusedInteraction OnInteract;
    UFUNCTION(BlueprintPure, Category="Interaction") bool CanInteract(ATripoCharacter* Player) const;
    UFUNCTION(BlueprintCallable, Category="Interaction") bool TryInteract(ATripoCharacter* Player);
    void SetFocused(bool bFocused);
private:
    UPROPERTY(Transient) TObjectPtr<UMaterialInterface> PreviousOverlay;
    bool bHighlighted = false;
};
