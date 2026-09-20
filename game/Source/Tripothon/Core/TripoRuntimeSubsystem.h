#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/TripoRuntimeState.h"
#include "TripoRuntimeSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTripoRuntime, Log, All);

// Single-player session lifetime. No Actor references are retained across maps.
UCLASS()
class TRIPOTHON_API UTripoRuntimeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    UFUNCTION(BlueprintPure, Category="Tripo|Session", meta=(WorldContext="WorldContextObject"))
    static UTripoRuntimeSubsystem* GetRuntime(const UObject* WorldContextObject);
    UFUNCTION(BlueprintCallable, Category="Tripo|Time") double GetActionSeconds();
    UFUNCTION(BlueprintCallable, Category="Tripo|Time") bool SetPauseReason(ETripoPauseReason Reason, bool bPaused);
    UFUNCTION(BlueprintPure, Category="Tripo|Time") bool HasPauseReason(ETripoPauseReason Reason) const;
    UFUNCTION(BlueprintPure, Category="Tripo|Time") bool IsActionPaused() const { return !Clock.Reasons.IsEmpty(); }
    UFUNCTION(BlueprintCallable, Category="Tripo|Recovery") bool AdvanceRestore(ETripoRestorePhase Next);
    UFUNCTION(BlueprintPure, Category="Tripo|Recovery") ETripoRestorePhase GetRestorePhase() const { return Restore.Phase; }
    UFUNCTION(BlueprintPure, Category="Tripo|Recovery") int64 GetEpoch() const { return Restore.Epoch; }
    UFUNCTION(BlueprintPure, Category="Tripo|Session") FGuid GetRunId() const { return RunId; }
    void BeginRun(FGuid Id) { if (Id.IsValid()) { RunId = Id; Restore = FTripoRestoreState(); } }
private:
    FTripoActionClock Clock;
    FTripoRestoreState Restore;
    FGuid RunId;
};
