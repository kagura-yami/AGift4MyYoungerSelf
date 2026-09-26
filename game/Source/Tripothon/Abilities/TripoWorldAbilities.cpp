#include "Abilities/TripoWorldAbilities.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Abilities/TripoStone.h"
#include "Player/TripoCharacter.h"
#include "World/TripoZone.h"
#include "World/TripoMechanism.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/CapsuleComponent.h"

static ATripoCharacter* AbilityPlayer(const UObject* Ability)
{ const auto* C = Cast<UTripoAbilityComponent>(Ability->GetOuter()); return C ? Cast<ATripoCharacter>(C->GetOwner()) : nullptr; }
ETripoAbilityFailure UTripoStoneAbility::CheckPlacement(ATripoCharacter* Player, const FTripoAbilityParameters& Parameters, FVector& Location)
{
    if (!IsValid(Player)) return ETripoAbilityFailure::InvalidContext;
    auto* World = Player->GetWorld();
    Location = Player->GetActorLocation() - FVector(0,0,Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight())
        + FRotator(0, Player->GetCameraYaw(), 0).RotateVector(Player->StonePlacementOffset);
    bool bHasAllowedZone = false;
    for (TActorIterator<ATripoZone> It(World); It; ++It) if (It->Kind == ETripoZoneKind::BuildAllowed) bHasAllowedZone = true;
    int32 Count = 0;
    for (TActorIterator<ATripoStone> It(World); It; ++It) if (It->GetOwner() == Player) ++Count;
    if (Count >= Parameters.Capacity) return ETripoAbilityFailure::Capacity;
    for (int32 X : {-1,1}) for (int32 Y : {-1,1}) for (int32 Z : {-1,1})
    {
        const FVector Corner = Location + FVector(X*75, Y*75, Z*12.5);
        if ((bHasAllowedZone && !ATripoZone::Inside(World, ETripoZoneKind::BuildAllowed, Corner)) || ATripoZone::Inside(World, ETripoZoneKind::BuildForbidden, Corner)) return ETripoAbilityFailure::Blocked;
    }
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoStone));
    if (World->OverlapBlockingTestByChannel(Location, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeBox(FVector(76,76,13.5)), Query)) return ETripoAbilityFailure::Blocked;
    return ETripoAbilityFailure::None;
}
ETripoAbilityFailure UTripoStoneAbility::Validate(AActor*, const FTripoAbilityParameters& P) const { FVector L; return CheckPlacement(AbilityPlayer(this), P, L); }
ETripoAbilityFailure UTripoStoneAbility::BeginEffect(AActor*, const FTripoAbilityParameters& P)
{
    auto* Player = AbilityPlayer(this); FVector Location;
    const auto Failure = CheckPlacement(Player, P, Location); if (Failure != ETripoAbilityFailure::None) return Failure;
    auto* Stone = Player->GetWorld()->SpawnActorDeferred<ATripoStone>(ATripoStone::StaticClass(), FTransform(Location), Player, Player, ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding);
    if (!Stone) return ETripoAbilityFailure::Blocked;
    Stone->Duration = P.Duration; Stone->FinishSpawning(FTransform(Location));
    return ETripoAbilityFailure::None;
}
ETripoAbilityFailure UTripoSlowAbility::Validate(AActor* Target, const FTripoAbilityParameters& P) const
{
    auto* Player = AbilityPlayer(this); auto* M = Cast<ATripoMechanism>(Target);
    if (!Player || !IsValid(M) || !M->bTimeAffectable || M->Kind != ETripoMechanismKind::Platform) return ETripoAbilityFailure::NoTarget;
    if (FVector::DistSquared(Player->GetActorLocation(), M->GetActorLocation()) > FMath::Square(P.Distance)) return ETripoAbilityFailure::NoTarget;
    FHitResult Hit; FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoSlow), false, Player);
    if (Player->GetWorld()->LineTraceSingleByChannel(Hit, Player->GetActorLocation(), M->GetActorLocation(), ECC_Visibility, Query) && Hit.GetActor() != M) return ETripoAbilityFailure::Blocked;
    return ETripoAbilityFailure::None;
}
ETripoAbilityFailure UTripoSlowAbility::BeginEffect(AActor* Target, const FTripoAbilityParameters& P)
{
    const auto Failure = Validate(Target, P); if (Failure != ETripoAbilityFailure::None) return Failure;
    Mechanism = Cast<ATripoMechanism>(Target); SlowHandle = Mechanism->AddSlow(this, P.Strength, P.Duration);
    return SlowHandle.IsValid() ? ETripoAbilityFailure::None : ETripoAbilityFailure::Blocked;
}
void UTripoSlowAbility::UpdateEffect(double) { if (!Mechanism.IsValid()) RequestCompletion(); }
void UTripoSlowAbility::EndEffect(bool) { if (Mechanism.IsValid()) Mechanism->RemoveSlow(SlowHandle); Mechanism.Reset(); SlowHandle.Invalidate(); }
