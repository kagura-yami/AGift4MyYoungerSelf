#include "World/TripoWorldSubsystem.h"
#include "World/TripoInteractorComponent.h"
#include "Time/TripoHistoryComponent.h"
#include "Player/TripoCharacter.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"

UTripoWorldSubsystem* UTripoWorldSubsystem::Get(const UObject* Context)
{ return Context && Context->GetWorld() ? Context->GetWorld()->GetSubsystem<UTripoWorldSubsystem>() : nullptr; }
bool UTripoWorldSubsystem::IsSafeSpawn(ATripoCharacter* Player, const FVector& Location) const
{
    if (!IsValid(Player) || Location.ContainsNaN()) return false;
    const auto* Capsule = Player->GetCapsuleComponent();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoSpawn), false, Player);
    if (GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity, ECC_Pawn,
        FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()), Query)) return false;
    FHitResult Floor;
    const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
    if (!GetWorld()->LineTraceSingleByChannel(Floor, Location, Location - FVector(0,0,HalfHeight + 100), ECC_Visibility, Query) || Floor.ImpactNormal.Z < .7f) return false;
    return Floor.GetActor() && !Floor.GetActor()->ActorHasTag(TEXT("TripoTemporary")) && !Cast<ATripoMechanism>(Floor.GetActor());
}
bool UTripoWorldSubsystem::SetCheckpoint(ATripoCharacter* Player, FTransform Transform, bool bChallenge)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || R->GetRestorePhase() != ETripoRestorePhase::Running || !IsSafeSpawn(Player, Transform.GetLocation())) return false;
    FTripoCheckpoint New;
    New.Player = Transform; New.bValid = true; New.Cooldowns = Player->Abilities->ExportCooldowns();
    for (TActorIterator<ATripoMechanism> It(GetWorld()); It; ++It) New.Mechanisms.Add(It->Identity->GetStableId(), It->Capture());
    Checkpoint = New;
    if (bChallenge) ChallengeStart = New;
    return true;
}
bool UTripoWorldSubsystem::RestorePlayer(ATripoCharacter* Player, bool bRestart)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    const FTripoCheckpoint& Snapshot = bRestart ? ChallengeStart : Checkpoint;
    if (!R || !IsValid(Player) || !Snapshot.bValid) { LastFailure = TEXT("No valid checkpoint"); return false; }
    if (R->GetRestorePhase() != ETripoRestorePhase::Running) return false;
    R->AdvanceRestore(ETripoRestorePhase::Locked);
    Player->Interactor->bSuppressed = true;
    Player->GetCharacterMovement()->StopMovementImmediately();
    Player->GetCharacterMovement()->DisableMovement();
    Player->Abilities->CancelAll();
    for (TActorIterator<AActor> It(GetWorld()); It; ++It) if (It->ActorHasTag(TEXT("TripoTemporary"))) It->Destroy();
    R->AdvanceRestore(ETripoRestorePhase::AbilitiesCleared);
    for (TActorIterator<ATripoMechanism> It(GetWorld()); It; ++It)
        if (const auto* State = Snapshot.Mechanisms.Find(It->Identity->GetStableId())) It->Restore(*State);
    R->AdvanceRestore(ETripoRestorePhase::WorldRestored);
    TArray<FTransform> Candidates = {Snapshot.Player, Checkpoint.Player};
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It) Candidates.Add(It->GetActorTransform());
    bool bPlaced = false;
    for (const auto& Candidate : Candidates)
    {
        if (!IsSafeSpawn(Player, Candidate.GetLocation())) continue;
        if (Player->TeleportTo(Candidate.GetLocation(), Candidate.Rotator(), false, true)) { bPlaced = true; break; }
    }
    if (!bPlaced)
    {
        // Retain the lock: unlocking a character inside restored geometry is unsafe.
        LastFailure = TEXT("Checkpoint and all fallback spawns obstructed; reload the safe save");
        return false;
    }
    Player->ResetAfterRestore();
    if (bRestart) Player->Abilities->ImportCooldowns(Snapshot.Cooldowns);
    R->AdvanceRestore(ETripoRestorePhase::PlayerRestored);
    for (TActorIterator<AActor> It(GetWorld()); It; ++It) if (auto* H = It->FindComponentByClass<UTripoHistoryComponent>()) H->Clear();
    R->AdvanceRestore(ETripoRestorePhase::HistoryCleared);
    Player->Interactor->bSuppressed = false;
    Player->GetCapsuleComponent()->UpdateOverlaps();
    for (TActorIterator<ATripoMechanism> It(GetWorld()); It; ++It) It->RefreshOccupants();
    for (TActorIterator<ATripoMechanism> It(GetWorld()); It; ++It) It->RefreshGate();
    R->AdvanceRestore(ETripoRestorePhase::OverlapsRefreshed);
    R->AdvanceRestore(ETripoRestorePhase::Running);
    LastFailure.Empty();
    return true;
}
