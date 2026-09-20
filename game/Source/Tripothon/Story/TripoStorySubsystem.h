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
    UFUNCTION(BlueprintPure) bool HasDialogue() const { return bOpen; }
    UFUNCTION(BlueprintPure) FName GetCurrentId() const { return bOpen ? Current.Id : NAME_None; }
    const FTripoStoryEvent& GetCurrent() const { return Current; }
private:
    UPROPERTY() FTripoStoryEvent Current;
    TWeakObjectPtr<ATripoCharacter> Reader;
    bool bOpen = false;
};
