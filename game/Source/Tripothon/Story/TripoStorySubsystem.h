#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Story/TripoStoryCatalog.h"
#include "TripoStorySubsystem.generated.h"
class ATripoCharacter;
UCLASS()
class TRIPOTHON_API UTripoStorySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, meta=(WorldContext="Context")) static UTripoStorySubsystem* Get(const UObject* Context);
    UFUNCTION(BlueprintCallable) bool OpenEvent(ATripoCharacter* Player, UTripoStoryCatalog* Catalog, FName Id);
    UFUNCTION(BlueprintCallable) void CloseEvent(bool bSkip);
    UFUNCTION(BlueprintCallable) void AdvanceDialogue();
    int32 GetLineIndex() const { return LineIndex; }
    bool IsLastLine() const { return LineIndex >= Lines.Num()-1; }
    FString GetLine() const { return Lines.IsValidIndex(LineIndex) ? Lines[LineIndex] : FString(); }
    UFUNCTION(BlueprintPure) bool HasDialogue() const { return bOpen; }
    void SetWorldSpeaker(AActor* Speaker) { WorldSpeaker=Speaker; }
    AActor* GetWorldSpeaker() const { return bOpen ? WorldSpeaker.Get() : nullptr; }
    UFUNCTION(BlueprintPure) FName GetCurrentId() const { return bOpen ? Current.Id : NAME_None; }
    const FTripoStoryEvent& GetCurrent() const { return Current; }
private:
    UPROPERTY() FTripoStoryEvent Current;
    TWeakObjectPtr<ATripoCharacter> Reader;
    TWeakObjectPtr<AActor> WorldSpeaker;
    bool bOpen = false;
    TArray<FString> Lines;
    int32 LineIndex = 0;
};
