#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Progress/TripoChallengeDefinition.h"
#include "Progress/TripoGiftReceipt.h"
#include "TripoGiftBox.generated.h"
class ATripoCharacter;
class UStaticMeshComponent;
class UTextRenderComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTripoGiftOpened,const FTripoGiftReceipt&,Receipt);

/** Single-player persistent reward pickup. Art is replaceable; the receipt belongs to the run. */
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoGiftBox : public AActor
{
    GENERATED_BODY()
public:
    ATripoGiftBox();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Gift") TObjectPtr<UStaticMeshComponent> BoxMesh;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Gift") TObjectPtr<UStaticMeshComponent> LidMesh;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Gift") TObjectPtr<UTextRenderComponent> Prompt;
    // Unique within a map. Give instances explicit IDs; identical IDs intentionally share a receipt.
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Gift") FName GiftId;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Gift") TArray<FTripoRewardOption> Rewards;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Gift") bool bEnabled=true;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Gift",meta=(ClampMin="50")) float InteractionDistance=240;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Gift",meta=(ClampMin="0")) float LidLift=85;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Gift",meta=(ClampMin=".05")) float OpenSeconds=.65f;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Gift") bool bOpened=false;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Gift") FTripoGiftReceipt Receipt;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Gift") FString LastError;
    UPROPERTY(BlueprintAssignable,Category="Gift") FTripoGiftOpened OnGiftOpened;
    UFUNCTION(BlueprintCallable,Category="Gift") bool TryOpen(ATripoCharacter* Player);
    UFUNCTION(BlueprintPure,Category="Gift") FName GetReceiptKey() const;
    UFUNCTION(BlueprintPure,Category="Gift") bool IsInReach(ATripoCharacter* Player) const;
    UFUNCTION(CallInEditor,Category="Gift") void GenerateNewGiftId();
    static ATripoGiftBox* FindNearby(ATripoCharacter* Player);
private:
    FVector ClosedLidLocation;
    FRotator ClosedLidRotation;
    float OpenAlpha=0;
    double OpenPresentationStart=0;
    void RefreshReceipt(bool bInstant);
    void UpdateVisuals();
};
