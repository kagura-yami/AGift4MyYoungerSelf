#include "Abilities/TripoAbilityDefinition.h"
#include "Abilities/TripoAbilityInstance.h"
#include "Abilities/TripoMovementAbilities.h"
#include "Abilities/TripoWorldAbilities.h"
#include "Abilities/TripoRewindAbility.h"
#include "Abilities/TripoEchoAbility.h"

bool UTripoAbilityDefinition::IsValidDefinition() const
{
    if (static_cast<uint8>(Ability) > static_cast<uint8>(ETripoAbility::BonusTime) || Levels.Num() != 3) return false;
    for (const auto& P : Levels)
        if (!FMath::IsFinite(P.Cooldown) || P.Cooldown < 0 || !FMath::IsFinite(P.Duration) || P.Duration < 0 ||
            !FMath::IsFinite(P.Distance) || P.Distance < 0 || !FMath::IsFinite(P.Strength) || P.Strength < 0 || P.Capacity < 1) return false;
    return true;
}
const FTripoAbilityParameters* UTripoAbilityDefinition::Parameters(int32 Level) const
{
    return IsValidDefinition() && Level >= 1 && Level <= 3 ? &Levels[Level - 1] : nullptr;
}
UTripoAbilityDefinition* UTripoAbilityDefinition::MakeDefaults(UObject* Outer, ETripoAbility Id)
{
    auto* D = NewObject<UTripoAbilityDefinition>(Outer);
    D->Ability = Id;
    if (Id == ETripoAbility::Dash) D->Implementation = UTripoDashAbility::StaticClass();
    if (Id == ETripoAbility::UpDash) D->Implementation = UTripoUpDashAbility::StaticClass();
    if (Id == ETripoAbility::WallJump) D->Implementation = UTripoWallJumpAbility::StaticClass();
    if (Id == ETripoAbility::StepStone) D->Implementation = UTripoStoneAbility::StaticClass();
    if (Id == ETripoAbility::Slow) D->Implementation = UTripoSlowAbility::StaticClass();
    if (Id == ETripoAbility::Rewind) D->Implementation = UTripoRewindAbility::StaticClass();
    if (Id == ETripoAbility::Echo) D->Implementation = UTripoEchoAbility::StaticClass();
    D->bRequiresTarget = Id == ETripoAbility::Slow;
    for (int32 Level = 1; Level <= 3; ++Level)
    {
        FTripoAbilityParameters P;
        // Editable graybox proposals, not final balance. Strength units depend on ability.
        switch (Id)
        {
        case ETripoAbility::Dash: P.Distance = 350 + 100 * Level; P.Duration = .2f; P.Cooldown = 1.6f - .2f * Level; break;
        case ETripoAbility::UpDash: P.Strength = 600 + 100 * Level; P.Duration = .2f; P.Cooldown = 2.f; break;
        case ETripoAbility::WallJump: P.Strength = 600 + 50 * Level; P.Distance = 60; P.Cooldown = .2f; break;
        case ETripoAbility::StepStone: P.Distance = 200; P.Duration = 6 + 2 * Level; P.Cooldown = 2; P.Capacity = Level; break;
        case ETripoAbility::Slow: P.Distance = 600; P.Duration = 2 + Level; P.Strength = .6f - .1f * Level; P.Cooldown = 8; break;
        case ETripoAbility::Rewind: P.Duration = 1 + 2 * Level; P.Cooldown = 10; break;
        case ETripoAbility::Echo: P.Duration = 2 + Level; P.Cooldown = 10; break;
        case ETripoAbility::BonusTime: P.Strength = 5 * Level; break;
        }
        D->Levels.Add(P);
    }
    return D;
}
