#include "Abilities/TripoMovementAbilities.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Player/TripoCharacter.h"
#include "Player/TripoMovementComponent.h"
#include "World/TripoTeleportPoint.h"
ATripoCharacter* UTripoDashAbility::Player() const { const auto* C = Cast<UTripoAbilityComponent>(GetOuter()); return C ? Cast<ATripoCharacter>(C->GetOwner()) : nullptr; }
ETripoAbilityFailure UTripoDashAbility::Validate(AActor*, const FTripoAbilityParameters&) const
{ auto* P = Player(); return P ? CastChecked<UTripoMovementComponent>(P->GetCharacterMovement())->CanBurst(IsUp()) : ETripoAbilityFailure::InvalidContext; }
ETripoAbilityFailure UTripoDashAbility::BeginEffect(AActor*, const FTripoAbilityParameters& Parameters)
{
    auto* P = Player(); if (!P) return ETripoAbilityFailure::InvalidContext;
    if (!IsUp() && P->HandleForwardDisplacement()) return ETripoAbilityFailure::None;
    FVector Direction = P->GetLastMovementInputVector().GetSafeNormal2D();
    if (Direction.IsNearlyZero()) Direction = P->GetActorForwardVector();
    return CastChecked<UTripoMovementComponent>(P->GetCharacterMovement())->BeginBurst(IsUp(), Direction, IsUp() ? Parameters.Strength : Parameters.Distance, Parameters.Duration) ? ETripoAbilityFailure::None : ETripoAbilityFailure::Blocked;
}
void UTripoDashAbility::UpdateEffect(double)
{
    auto* P = Player(); if (!P) { RequestCompletion(); return; }
    auto* M = CastChecked<UTripoMovementComponent>(P->GetCharacterMovement());
    if (!IsUp() && M->IsBursting()) P->HandleForwardDisplacement();
    if (!M->IsBursting())
    {
        if (M->BurstHit.bBlockingHit) P->Abilities->ReportHit(GetHandle(), M->BurstHit);
        RequestCompletion();
    }
}
void UTripoDashAbility::EndEffect(bool) { if (auto* P = Player()) CastChecked<UTripoMovementComponent>(P->GetCharacterMovement())->EndBurst(); }
ETripoAbilityFailure UTripoWallJumpAbility::Validate(AActor*, const FTripoAbilityParameters&) const
{ return ETripoAbilityFailure::None; }
ETripoAbilityFailure UTripoWallJumpAbility::BeginEffect(AActor*, const FTripoAbilityParameters& Parameters)
{
    auto* C = Cast<UTripoAbilityComponent>(GetOuter()); auto* P = C ? Cast<ATripoCharacter>(C->GetOwner()) : nullptr;
    return P ? CastChecked<UTripoMovementComponent>(P->GetCharacterMovement())->WallJump(Parameters.Strength, Parameters.Distance) : ETripoAbilityFailure::InvalidContext;
}
