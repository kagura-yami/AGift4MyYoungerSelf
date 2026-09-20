#include "Time/TripoHistoryComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoIdentityRegistry.h"
#include "World/TripoMechanism.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
UTripoHistoryComponent::UTripoHistoryComponent() { PrimaryComponentTick.bCanEverTick = true; }
void UTripoHistoryComponent::Clear() { Frames.Empty(); Events.Empty(); LastSample = -1; }
void UTripoHistoryComponent::TickComponent(float Delta, ELevelTick TickType, FActorComponentTickFunction* Function)
{
    Super::TickComponent(Delta, TickType, Function);
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this); if (!R || !GetOwner()) return;
    if (Epoch != R->GetEpoch()) { Clear(); Epoch = R->GetEpoch(); }
    const double Now = R->GetActionSeconds();
    if (bReplaying || R->IsActionPaused() || R->GetRestorePhase() != ETripoRestorePhase::Running || Now - LastSample < 1./30.) return;
    FTripoHistoryFrame Frame; Frame.Time = Now; Frame.Transform = GetOwner()->GetActorTransform(); Frame.Velocity = GetOwner()->GetVelocity();
    if (auto* Character = Cast<ACharacter>(GetOwner()))
        if (auto* Base = Character->GetMovementBase())
            if (auto* Id = Base->GetOwner()->FindComponentByClass<UTripoIdentityComponent>())
            { Frame.BaseId = Id->GetStableId(); Frame.Relative = Frame.Transform.GetRelativeTransform(Base->GetOwner()->GetActorTransform()); }
    if (auto* M = Cast<ATripoMechanism>(GetOwner())) Frame.Phase = M->Capture().Phase;
    Frames.Add(Frame); LastSample = Now;
    while (Frames.Num() > 240 || (Frames.Num() > 1 && Frames[1].Time < Now - 7.)) Frames.RemoveAt(0);
    Events.RemoveAll([Now](const auto& E) { return E.Time < Now - 7.; });
}
TArray<FTripoHistoryFrame> UTripoHistoryComponent::Clip(double Seconds) const
{
    TArray<FTripoHistoryFrame> Result; if (Frames.Num() < 2 || Seconds <= 0) return Result;
    const double Start = FMath::Max(Frames[0].Time, Frames.Last().Time - FMath::Min(7., Seconds));
    FTripoHistoryFrame First; Sample(Frames, Start, First); Result.Add(First);
    for (const auto& F : Frames) if (F.Time > Start) Result.Add(F);
    return Result;
}
bool UTripoHistoryComponent::Sample(const TArray<FTripoHistoryFrame>& Clip, double Time, FTripoHistoryFrame& Frame)
{
    if (Clip.IsEmpty()) return false;
    if (Time <= Clip[0].Time) { Frame = Clip[0]; return true; }
    for (int32 I = 1; I < Clip.Num(); ++I)
        if (Clip[I].Time >= Time)
        {
            const auto& A = Clip[I-1]; const auto& B = Clip[I];
            const float Alpha = float((Time-A.Time) / FMath::Max(1.e-9, B.Time-A.Time));
            Frame = A; Frame.Time = Time; Frame.Transform.Blend(A.Transform, B.Transform, Alpha);
            Frame.Velocity = FMath::Lerp(A.Velocity, B.Velocity, Alpha);
            if (A.BaseId == B.BaseId) Frame.Relative.Blend(A.Relative, B.Relative, Alpha); else Frame.BaseId.Invalidate();
            // Phase uses wrapped interpolation across the platform cycle boundary.
            double Difference = B.Phase - A.Phase; if (Difference < -.5) Difference += 1; if (Difference > .5) Difference -= 1;
            Frame.Phase = FMath::Fmod(A.Phase + Difference * Alpha + 1., 1.); return true;
        }
    Frame = Clip.Last(); return true;
}
bool UTripoHistoryComponent::Resolve(FTripoHistoryFrame& Frame) const
{
    if (!Frame.BaseId.IsValid()) return true;
    auto* Base = UTripoIdentityRegistry::GetRegistry(this)->Resolve(Frame.BaseId);
    if (!IsValid(Base)) return false;
    Frame.Transform = Frame.Relative * Base->GetActorTransform(); return true;
}
void UTripoHistoryComponent::RecordInteraction(FGuid TargetId)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this); if (!R || bReplaying || !TargetId.IsValid()) return;
    FTripoHistoryEvent E; E.Time = R->GetActionSeconds(); E.TargetId = TargetId; E.EventId = FGuid::NewGuid();
    Events.Add(E); if (Events.Num() > 128) Events.RemoveAt(0);
}
TArray<FTripoHistoryEvent> UTripoHistoryComponent::ClipEvents(double Start, double End) const
{ TArray<FTripoHistoryEvent> Result; for (const auto& E : Events) if (E.Time >= Start && E.Time <= End) Result.Add(E); return Result; }
void UTripoHistoryComponent::PruneAfter(double Time)
{
    Frames.RemoveAll([Time](const auto& F) { return F.Time > Time; });
    Events.RemoveAll([Time](const auto& E) { return E.Time > Time; }); LastSample = -1;
}
