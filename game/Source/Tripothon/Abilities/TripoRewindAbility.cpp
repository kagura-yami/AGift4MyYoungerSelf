#include "Abilities/TripoRewindAbility.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Player/TripoCharacter.h"
#include "Player/TripoMovementComponent.h"
#include "World/TripoMechanism.h"
#include "World/TripoInteractorComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "EngineUtils.h"

static ATripoCharacter* RewindPlayer(const UObject* Instance)
{ auto* C = Cast<UTripoAbilityComponent>(Instance->GetOuter()); return C ? Cast<ATripoCharacter>(C->GetOwner()) : nullptr; }
ETripoAbilityFailure UTripoRewindAbility::Validate(AActor* Target, const FTripoAbilityParameters& P) const
{
    auto* Player = RewindPlayer(this); if (!Player) return ETripoAbilityFailure::InvalidContext;
    auto* M = Cast<ATripoMechanism>(Target);
    if (Target && (!IsValid(M) || M->Kind != ETripoMechanismKind::Platform || !M->bTimeAffectable || M->bRewinding || FVector::DistSquared(Player->GetActorLocation(), M->GetActorLocation()) > FMath::Square(600.))) return ETripoAbilityFailure::NoTarget;
    if (M)
    {
        FHitResult Hit; FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoRewindTarget), false, Player);
        if (Player->GetWorld()->LineTraceSingleByChannel(Hit, Player->GetActorLocation(), M->GetActorLocation(), ECC_Visibility, Query) && Hit.GetActor() != M) return ETripoAbilityFailure::Blocked;
    }
    if (!Target && CastChecked<UTripoMovementComponent>(Player->GetCharacterMovement())->IsBursting()) return ETripoAbilityFailure::Blocked;
    auto* H = (Target ? Target : Player)->FindComponentByClass<UTripoHistoryComponent>();
    const auto Clip = H ? H->Clip(P.Duration) : TArray<FTripoHistoryFrame>();
    return Clip.Num() >= 2 && Clip.Last().Time - Clip[0].Time >= .1 ? ETripoAbilityFailure::None : ETripoAbilityFailure::NoTarget;
}
ETripoAbilityFailure UTripoRewindAbility::BeginEffect(AActor* Target, const FTripoAbilityParameters& P)
{
    const auto Failure = Validate(Target, P); if (Failure != ETripoAbilityFailure::None) return Failure;
    auto* Player = RewindPlayer(this); Subject = Target ? Target : Player; History = Subject->FindComponentByClass<UTripoHistoryComponent>();
    Frames = History->Clip(P.Duration); Cursor = Frames.Last().Time; Length = Cursor - Frames[0].Time;
    Epoch = UTripoRuntimeSubsystem::GetRuntime(Player)->GetEpoch(); History->bReplaying = true;
    if (auto* M = Cast<ATripoMechanism>(Subject.Get())) M->bRewinding = true;
    else
    {
        for (TActorIterator<AActor> It(Player->GetWorld()); It; ++It) if (It->ActorHasTag(TEXT("TripoEcho")) && It->GetOwner() == Player) It->Destroy();
        Player->Interactor->bSuppressed = true;
        Player->GetCharacterMovement()->StopMovementImmediately(); Player->GetCharacterMovement()->SetMovementMode(MOVE_Custom, 2);
    }
    return ETripoAbilityFailure::None;
}
void UTripoRewindAbility::UpdateEffect(double Delta)
{
    if (!Subject.IsValid() || !History.IsValid() || Epoch != UTripoRuntimeSubsystem::GetRuntime(this)->GetEpoch()) { RequestCompletion(); return; }
    const double To = FMath::Max(Frames[0].Time, Cursor - Delta);
    TArray<double> Stops;
    for (int32 I = Frames.Num()-1; I >= 0; --I) if (Frames[I].Time < Cursor && Frames[I].Time > To) Stops.Add(Frames[I].Time);
    Stops.Add(To);
    for (double Time : Stops)
    {
        FTripoHistoryFrame F; UTripoHistoryComponent::Sample(Frames, Time, F);
        if (!History->Resolve(F)) { RequestCompletion(); return; }
        bool bMoved = false;
        if (auto* M = Cast<ATripoMechanism>(Subject.Get()))
        { FTripoMechanismState S; S.Transform = F.Transform; S.Phase = F.Phase; bMoved = M->ApplyRewindState(S); }
        else if (auto* P = Cast<ATripoCharacter>(Subject.Get())) bMoved = CastChecked<UTripoMovementComponent>(P->GetCharacterMovement())->ReplayTo(F.Transform, F.Velocity);
        if (!bMoved) { RequestCompletion(); return; }
        Cursor = Time;
    }
    if (Cursor <= Frames[0].Time) RequestCompletion();
}
void UTripoRewindAbility::EndEffect(bool)
{
    if (History.IsValid()) { History->PruneAfter(Cursor); History->bReplaying = false; }
    if (auto* M = Cast<ATripoMechanism>(Subject.Get())) M->bRewinding = false;
    else if (auto* P = Cast<ATripoCharacter>(Subject.Get()))
    {
        if (UTripoRuntimeSubsystem::GetRuntime(P)->GetRestorePhase() == ETripoRestorePhase::Running)
        { P->Interactor->bSuppressed = false; P->GetCharacterMovement()->SetMovementMode(MOVE_Falling); }
    }
    Frames.Empty(); Subject.Reset(); History.Reset();
}
