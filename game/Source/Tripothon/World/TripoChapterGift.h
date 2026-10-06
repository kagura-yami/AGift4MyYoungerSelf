#pragma once
#include "CoreMinimal.h"
#include "World/TripoGiftBox.h"
#include "TripoChapterGift.generated.h"
class UMaterialInstanceDynamic;
class UTripoInteractionTarget;
/** Local chapter reward; its saved gift receipt also unlocks the doorway. */
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoChapterGift : public ATripoGiftBox
{
    GENERATED_BODY()
public:
    ATripoChapterGift();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual bool TryOpen(ATripoCharacter* Player) override;
    UFUNCTION(BlueprintCallable) bool ChooseSkill(ATripoCharacter* Player,int32 Ability);
    UFUNCTION(BlueprintPure) TArray<int32> GetChoices() const;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Portal;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName Destination;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName CompletionEvent = TEXT("Story.Home.GiftFound");
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FString ChapterCaption = TEXT("第一章  /  家");
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FString ExitHint = TEXT("收好礼物后，从左侧的门去学校");
    UPROPERTY(EditAnywhere,BlueprintReadWrite) float PortalScale = 2.6f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bScreenPortal = false;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) bool bFadeToRoom = false;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite) TObjectPtr<AActor> ArrivalPoint;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UTripoInteractionTarget> PortalInteraction;
    bool CanUsePortal() const;
    bool EnterPortal(ATripoCharacter* Player);
private:
    TWeakObjectPtr<ATripoCharacter> ChoosingPlayer;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> PortalMaterial;
    bool bPortalReady=false;
    bool bEntryArmed=false;
    bool bTravelling=false;
    float Reveal=0;
    float RoomFade=-1;
    bool bRoomArrived=false;
    double RetryAfter=0;
    void RefreshPortal(float DeltaSeconds);
};
