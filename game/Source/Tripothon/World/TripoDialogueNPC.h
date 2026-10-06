#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoDialogueNPC.generated.h"
class USkeletalMeshComponent;
class UCapsuleComponent;
class UTripoInteractionTarget;
class UTripoStoryCatalog;
class UAnimSequence;
class ATripoCharacter;
class UMediaSource;
class UMaterialInterface;

UCLASS(Blueprintable)
class TRIPOTHON_API ATripoDialogueNPC : public AActor
{
    GENERATED_BODY()
public:
    ATripoDialogueNPC();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<USkeletalMeshComponent> Visual;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UCapsuleComponent> Body;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UTripoInteractionTarget> Interaction;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dialogue") TObjectPtr<UTripoStoryCatalog> DialogueCatalog;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dialogue") FName DialogueEvent;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dialogue|Animation") TObjectPtr<UAnimSequence> IdleAnimation;
    /** Optional. Assign the generated skeletal animation here when ready. */
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dialogue|Animation") TObjectPtr<UAnimSequence> TalkAnimation;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dialogue|Ending Movie") TObjectPtr<UMediaSource> EndingMovie;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Dialogue|Ending Movie") TObjectPtr<UMaterialInterface> EndingMovieMaterial;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Dialogue") int32 ConversationCount=0;
    UFUNCTION(BlueprintImplementableEvent,Category="Dialogue") void OnConversationStarted(ATripoCharacter* Player);
private:
    UFUNCTION() void Talk(ATripoCharacter* Player);
    void RestoreIdle();
    void PlaySequence(UAnimSequence* Animation,bool bLoop);
    FTimerHandle AnimationTimer;
};
