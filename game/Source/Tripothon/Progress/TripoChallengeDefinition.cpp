#include "Progress/TripoChallengeDefinition.h"
UTripoChallengeDefinition::UTripoChallengeDefinition() { RequiredLevels.Init(0, 8); }
bool UTripoChallengeDefinition::IsValidDefinition() const
{
    if (ChallengeId.IsNone() || !FMath::IsFinite(Budget) || Budget < 0 || RequiredLevels.Num() != 8) return false;
    for (int32 L : RequiredLevels) if (L < 0 || L > 3) return false;
    TSet<ETripoAbility> Seen;
    for (const auto& R : Rewards)
    {
        if (uint8(R.Ability) >= 8 || !FMath::IsFinite(R.Weight) || R.Weight <= 0 || Seen.Contains(R.Ability)) return false;
        Seen.Add(R.Ability);
    }
    return true;
}
