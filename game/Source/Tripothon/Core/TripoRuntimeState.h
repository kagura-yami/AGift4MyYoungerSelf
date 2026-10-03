#pragma once
#include "CoreMinimal.h"
#include "TripoRuntimeState.generated.h"

UENUM(BlueprintType)
enum class ETripoPauseReason : uint8 { Menu, Loading, Reward };

UENUM(BlueprintType)
enum class ETripoRestorePhase : uint8
{
    Running, Locked, AbilitiesCleared, WorldRestored, PlayerRestored, HistoryCleared, OverlapsRefreshed
};

// Clock accepts a monotonic sample, never world delta time or time dilation.
struct FTripoActionClock
{
    double Seconds = 0.;
    double LastSample = 0.;
    bool bInitialized = false;
    TSet<ETripoPauseReason> Reasons;
    void Sample(double Now);
    void SetPaused(ETripoPauseReason Reason, bool bPaused, double Now);
};

struct FTripoRestoreState
{
    ETripoRestorePhase Phase = ETripoRestorePhase::Running;
    int64 Epoch = 0;
    bool Advance(ETripoRestorePhase Next);
};
