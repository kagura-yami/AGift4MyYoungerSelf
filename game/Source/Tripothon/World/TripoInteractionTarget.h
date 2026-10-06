#pragma once
#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "TripoInteractionTarget.generated.h"
class ATripoCharacter;
class UMeshComponent;
class UMaterialInterface;
class UTripoStoryCatalog;
class UWidgetComponent;
class UTripoNPCLookAnimInstance;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTripoFocusedInteraction, ATripoCharacter*, Player);
/** A single-player, aim-selected interaction surface. Attach to an object's visible control. */
UCLASS(ClassGroup=(Tripo), meta=(BlueprintSpawnableComponent))
class TRIPOTHON_API UTripoInteractionTarget : public UBoxComponent
{
    GENERATED_BODY()
public:
    UTripoInteractionTarget();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") bool bUseSchoolKey = false;
    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Interaction") bool bSchoolDoorUnlocked = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") FText Prompt;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") float Reach = 240;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") TObjectPtr<UMeshComponent> HighlightMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") TObjectPtr<UMaterialInterface> HighlightMaterial;
    UPROPERTY(BlueprintAssignable, Category="Interaction") FTripoFocusedInteraction OnInteract;
    /** Optional parameterless Blueprint event for existing authored mechanisms. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") FName InteractionEvent;
    /** Optional conversation using the shared non-blocking story bubble. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Dialogue") TObjectPtr<UTripoStoryCatalog> StoryCatalog;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Dialogue") FName StoryEvent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Dialogue") bool bNPCConversation = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") bool bSingleUse = false;
    /** Existing door Blueprint must own one timeline that drives the door leaf. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction") bool bToggleDoorTimeline = false;
    UFUNCTION(BlueprintPure, Category="Interaction") bool CanInteract(ATripoCharacter* Player) const;
    UFUNCTION(BlueprintCallable, Category="Interaction") bool TryInteract(ATripoCharacter* Player);
    void SetFocused(bool bFocused);
private:
    UPROPERTY(Transient) TObjectPtr<UWidgetComponent> SpeechBubble;
    UPROPERTY(Transient) TObjectPtr<UTripoNPCLookAnimInstance> LookAnimation;
    float BodyTurnDelay=0.f;
    float BodyTurnSpeed=0.f;
    bool bBodyTurning=false;
    TWeakObjectPtr<ATripoCharacter> ConversationPlayer;
    float BubbleOpacity = 0.f;
    UPROPERTY(Transient) TObjectPtr<UMaterialInterface> PreviousOverlay;
    UPROPERTY(Transient) TObjectPtr<UMaterialInterface> PreviousLidOverlay;
    bool bHighlighted = false;
};
