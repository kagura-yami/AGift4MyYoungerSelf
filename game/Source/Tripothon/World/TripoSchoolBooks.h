#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoSchoolBooks.generated.h"
class UTripoInteractionTarget;
class UStaticMeshComponent;
class UWidgetComponent;
class UTexture2D;
class USoundBase;
class ATripoCharacter;
class ATripoSchoolBooks;

/** Authored pickups owned by one room puzzle. Runtime state lasts for this map visit. */
UCLASS()
class TRIPOTHON_API ATripoSchoolBookItem : public AActor
{
    GENERATED_BODY()
public:
    ATripoSchoolBookItem();
    virtual void BeginPlay() override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UTripoInteractionTarget> Interaction;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<ATripoSchoolBooks> Puzzle;
    /** 0 Chinese, 1 maths, 2 English, 3 reward key. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Subject = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bCollected = false;
private:
    UFUNCTION() void Collect(ATripoCharacter* Player);
};

UCLASS()
class TRIPOTHON_API ATripoSchoolBooks : public AActor
{
    GENERATED_BODY()
public:
    ATripoSchoolBooks();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TObjectPtr<ATripoSchoolBookItem>> Books;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<ATripoSchoolBookItem> RewardKey;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<AActor> LockedDoor;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<AActor> Drawer;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector DrawerTravel = FVector(0,35,0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TObjectPtr<UTexture2D>> Icons;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TObjectPtr<USoundBase>> Notes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<USoundBase> CompletionSound;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UWidgetComponent> Instruction;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UWidgetComponent> CheckChinese;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UWidgetComponent> CheckMath;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UWidgetComponent> CheckEnglish;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 CollectedCount = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bKeyCollected = false;
    bool TryCollect(ATripoSchoolBookItem* Item, ATripoCharacter* Player);
    FString GetHint() const;
private:
    FString Hint;
    double HintUntil = 0;
    float RevealTime = -1;
    FVector DrawerStart;
    FVector KeyStart;
    void Say(const FString& Text);
};
