#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoOfficeCipher.generated.h"
class UStaticMeshComponent;
class UTripoInteractionTarget;
class ATripoCharacter;
class ATripoChapterGift;
class UMaterialInterface;
UCLASS()
class TRIPOTHON_API ATripoOfficeCipher : public AActor
{
    GENERATED_BODY()
public:
    ATripoOfficeCipher();
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Card;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Book;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Drawer;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UTripoInteractionTarget> CardTarget;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UTripoInteractionTarget> BookTarget;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UTripoInteractionTarget> DrawerTarget;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TObjectPtr<ATripoChapterGift> Reward;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TObjectPtr<UMaterialInterface> CardUI;
    /** Place a BP_CipherArrival in the level. Its origin is the player's capsule centre. */
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Cipher|Teleport") TObjectPtr<AActor> CodeDestination;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Cipher") FVector2D CardOffset=FVector2D::ZeroVector;
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector DrawerTravel=FVector(0,35,0);
    UPROPERTY(EditAnywhere,BlueprintReadWrite) FName PuzzleId=TEXT("Company.Cipher");
    // Four printed digits per page: top left, bottom left, top right, bottom right.
    UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<int32> PageDigits;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly) bool bHasCard=false;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly) bool bUnlocked=false;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly) int32 Page=1;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly) bool bCardOnPage=false;
    UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly) FString Feedback;
    UFUNCTION(BlueprintCallable) bool TakeCard(ATripoCharacter* Player);
    UFUNCTION(BlueprintCallable) bool OpenBook(ATripoCharacter* Player);
    UFUNCTION(BlueprintCallable) bool OpenLock(ATripoCharacter* Player);
    UFUNCTION(BlueprintCallable) void SelectPage(int32 Number);
    UFUNCTION(BlueprintCallable) void ToggleCard();
    UFUNCTION(BlueprintCallable) void MoveCard(FVector2D Offset);
    UFUNCTION(BlueprintCallable) bool SubmitCode(ATripoCharacter* Player,const FString& Code);
    UFUNCTION(BlueprintPure) int32 Digit(int32 PageNumber,int32 Hole) const;
private:
    FVector DrawerClosed;
    FVector RewardClosed;
    float OpenAlpha=0;
    UFUNCTION() void CardUsed(ATripoCharacter* Player);
    UFUNCTION() void BookUsed(ATripoCharacter* Player);
    UFUNCTION() void DrawerUsed(ATripoCharacter* Player);
    bool InReach(ATripoCharacter* Player,UTripoInteractionTarget* Target) const;
    void RefreshObjects();
};
