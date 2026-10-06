#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Progress/TripoChallengeDefinition.h"
#include "Progress/TripoGiftReceipt.h"
#include "TripoProgressSubsystem.generated.h"
class ATripoCharacter;
class UTripoSaveGame;
UENUM(BlueprintType)
enum class ETripoChallengePhase : uint8 { Idle, Running, PendingReward, Committed };
UCLASS(Config=Game)
class TRIPOTHON_API UTripoProgressSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    UFUNCTION(BlueprintPure, meta=(WorldContext="Context")) static UTripoProgressSubsystem* Get(const UObject* Context);
    UFUNCTION(BlueprintCallable) bool Start(ATripoCharacter* Player, UTripoChallengeDefinition* Definition);
    UFUNCTION(BlueprintCallable) bool Finish(ATripoCharacter* Player, FName ChallengeId);
    bool FinishWithGifts(ATripoCharacter* Player, FName Id);
    int32 GetFinishGiftCount(FName Id) const;
    UFUNCTION(BlueprintCallable) bool CommitReward(ATripoCharacter* Player, int32 ChoiceIndex);
    UFUNCTION(BlueprintCallable) bool Restart(ATripoCharacter* Player);
    UFUNCTION(BlueprintCallable) double GetElapsed() const;
    UFUNCTION(BlueprintPure) ETripoChallengePhase GetPhase() const { return Phase; }
    UFUNCTION(BlueprintPure) bool HasCompleted(FName Id) const { return Completed.Contains(Id); }
    UFUNCTION(BlueprintPure) TArray<FTripoRewardOption> GetCandidates() const { return Candidates; }
    UFUNCTION(BlueprintPure) FString GetMessage() const { return Message; }
    UFUNCTION(BlueprintPure) double GetBudget() const { return EffectiveBudget; }
    UFUNCTION(BlueprintPure) bool IsChoiceEligible() const { return bChoice; }
    UFUNCTION(BlueprintPure) FName GetChallengeId() const { return CurrentId; }
    UFUNCTION(BlueprintCallable) bool Exchange(ATripoCharacter* Player, FGuid Transaction, ETripoAbility From, ETripoAbility To, const TArray<int32>& Floors);
    UFUNCTION(BlueprintCallable) bool SaveSafe(ATripoCharacter* Player);
    UFUNCTION(BlueprintCallable) bool ContinueGame(bool bLab);
    UFUNCTION(BlueprintCallable) void NewGame(bool bLab);
    UFUNCTION(BlueprintPure) bool HasCompatibleSave(bool bLab) const;
    UFUNCTION(BlueprintPure) FName GetChapterDestination(FName EventId) const;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Story|Flow") FString FirstChapterMap;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Story|Flow") FString SecondChapterMap;
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Story|Flow") FString ThirdChapterMap;
    UFUNCTION(BlueprintCallable) bool Travel(ATripoCharacter* Player, FName MapPackage);
    bool ApplyPendingLoad(ATripoCharacter* Player);
    bool HasPendingLoad() const { return PendingLoad != nullptr || !PendingTravelLevels.IsEmpty(); }
    bool IsLabWorld(const UObject* Context) const;
    UFUNCTION(BlueprintPure) bool HasViewed(FName Id) const { return Viewed.Contains(Id); }
    UFUNCTION(BlueprintPure) bool HasApplied(FName Id) const { return Applied.Contains(Id); }
    bool ConsumeEntryMenu() { if (bEntryShown) return false; bEntryShown = true; return true; }
    bool ApplyStory(ATripoCharacter* Player, FName Id, const TArray<int32>& Floors);
    void MarkViewed(FName Id) { Viewed.Add(Id); }
    UFUNCTION(BlueprintPure) int32 GetReplyChoice() const { return ReplyChoice; }
    bool SelectReply(int32 Choice);
    UFUNCTION(BlueprintPure, Category="Tripo|Gift") bool HasClaimedGift(FName Key) const { return Gifts.Contains(Key); }
    UFUNCTION(BlueprintPure, Category="Tripo|Gift") FTripoGiftReceipt GetGiftReceipt(FName Key) const { const auto* Value=Gifts.Find(Key); return Value ? *Value : FTripoGiftReceipt(); }
    // Called only by a validated nearby gift actor; not a Blueprint remote-award shortcut.
    bool ClaimGift(ATripoCharacter* Player, FName Key, const TArray<FTripoRewardOption>& Pool, FTripoGiftReceipt& OutReceipt);
    TArray<int32> GetChapterGiftChoices(FName Key) const;
    bool ClaimChapterGift(ATripoCharacter* Player,FName Key,int32 Ability,FTripoGiftReceipt& OutReceipt);
private:
    UPROPERTY() TMap<FName, FTripoGiftReceipt> Gifts;
    UPROPERTY() int32 ReplyChoice = INDEX_NONE;
    UPROPERTY() TArray<int32> PendingTravelLevels;
    UPROPERTY() FString PendingTravelMap;
    TWeakObjectPtr<UWorld> DepartingWorld;
    bool bEntryShown = false;
    UPROPERTY() TObjectPtr<UTripoSaveGame> PendingLoad;
    UPROPERTY() TSet<FName> Viewed;
    UPROPERTY() TSet<FName> Applied;
    int64 SaveSequence = 0;
    UPROPERTY() TObjectPtr<UTripoChallengeDefinition> Current;
    UPROPERTY() TSet<FName> Completed;
    UPROPERTY() TArray<FTripoRewardOption> Candidates;
    TSet<FGuid> Exchanges;
    ETripoChallengePhase Phase = ETripoChallengePhase::Idle;
    FName CurrentId;
    int32 RunSeed = 0;
    double StartedAt = 0;
    double FinishElapsed = 0;
    double EffectiveBudget = 0;
    int32 RandomIndex = INDEX_NONE;
    bool bChoice = false;
    bool bPractice = false;
    FString Message;
};
