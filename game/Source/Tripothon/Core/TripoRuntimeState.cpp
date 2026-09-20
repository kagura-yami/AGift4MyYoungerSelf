#include "Core/TripoRuntimeState.h"

void FTripoActionClock::Sample(double Now)
{
    if (!FMath::IsFinite(Now) || (bInitialized && Now < LastSample)) return;
    if (bInitialized && Reasons.IsEmpty()) Seconds += Now - LastSample;
    LastSample = Now;
    bInitialized = true;
}

void FTripoActionClock::SetPaused(ETripoPauseReason Reason, bool bPaused, double Now)
{
    Sample(Now);
    if (bPaused) Reasons.Add(Reason);
    else Reasons.Remove(Reason);
}

bool FTripoRestoreState::Advance(ETripoRestorePhase Next)
{
    const uint8 Expected = (static_cast<uint8>(Phase) + 1) % 7;
    if (static_cast<uint8>(Next) != Expected) return false;
    Phase = Next;
    if (Next == ETripoRestorePhase::HistoryCleared) ++Epoch;
    return true;
}
