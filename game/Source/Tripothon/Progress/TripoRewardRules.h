#pragma once
#include "CoreMinimal.h"
#include "Progress/TripoChallengeDefinition.h"

// Pure rules used by the game-thread transaction and boundary tests.
namespace TripoReward
{
    inline bool ChoiceEligible(double Elapsed, double Budget) { return FMath::IsFinite(Elapsed) && FMath::IsFinite(Budget) && Elapsed >= 0 && Elapsed <= Budget; }
    TRIPOTHON_API TArray<FTripoRewardOption> Filter(const TArray<FTripoRewardOption>& Pool, const TArray<int32>& Levels);
    TRIPOTHON_API int32 Draw(const TArray<FTripoRewardOption>& Pool, int32 Seed, FName Challenge);
}
