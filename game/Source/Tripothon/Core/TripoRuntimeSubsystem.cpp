#include "Core/TripoRuntimeSubsystem.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY(LogTripoRuntime);

UTripoRuntimeSubsystem* UTripoRuntimeSubsystem::GetRuntime(const UObject* WorldContextObject)
{
    auto* Instance = UGameplayStatics::GetGameInstance(WorldContextObject);
    return Instance ? Instance->GetSubsystem<UTripoRuntimeSubsystem>() : nullptr;
}

void UTripoRuntimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RunId = FGuid::NewGuid();
    Clock.Sample(FPlatformTime::Seconds());
}

double UTripoRuntimeSubsystem::GetActionSeconds()
{
    Clock.Sample(FPlatformTime::Seconds());
    return Clock.Seconds;
}

bool UTripoRuntimeSubsystem::HasPauseReason(ETripoPauseReason Reason) const
{
    return Clock.Reasons.Contains(Reason);
}

bool UTripoRuntimeSubsystem::SetPauseReason(ETripoPauseReason Reason, bool bPaused)
{
    if (Reason != ETripoPauseReason::Menu && Reason != ETripoPauseReason::Loading && Reason != ETripoPauseReason::Reward) return false;
    if (!GetWorld()) return false;
    const bool bWasSet = HasPauseReason(Reason);
    Clock.SetPaused(Reason, bPaused, FPlatformTime::Seconds());
    const bool bDesired = IsActionPaused();
    if (UGameplayStatics::IsGamePaused(GetWorld()) != bDesired && !UGameplayStatics::SetGamePaused(GetWorld(), bDesired))
    {
        Clock.SetPaused(Reason, bWasSet, FPlatformTime::Seconds());
        UE_LOG(LogTripoRuntime, Warning, TEXT("Pause rejected reason=%d"), int32(Reason));
        return false;
    }
    UE_LOG(LogTripoRuntime, Log, TEXT("Pause reason=%d set=%d active=%d action=%.6f"), int32(Reason), bPaused, bDesired, Clock.Seconds);
    return true;
}

bool UTripoRuntimeSubsystem::AdvanceRestore(ETripoRestorePhase Next)
{
    if (!Restore.Advance(Next))
    {
        UE_LOG(LogTripoRuntime, Warning, TEXT("Restore transition rejected %d -> %d"), int32(Restore.Phase), int32(Next));
        return false;
    }
    UE_LOG(LogTripoRuntime, Log, TEXT("Restore phase=%d epoch=%lld"), int32(Next), Restore.Epoch);
    return true;
}
