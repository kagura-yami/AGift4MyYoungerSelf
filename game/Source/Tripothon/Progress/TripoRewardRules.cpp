#include "Progress/TripoRewardRules.h"
TArray<FTripoRewardOption> TripoReward::Filter(const TArray<FTripoRewardOption>& Pool, const TArray<int32>& Levels)
{
    TArray<FTripoRewardOption> Result;
    TSet<ETripoAbility> Seen;
    for (const auto& R : Pool)
        if (Levels.IsValidIndex(uint8(R.Ability)) && Levels[uint8(R.Ability)] < 3 && FMath::IsFinite(R.Weight) && R.Weight > 0 && !Seen.Contains(R.Ability))
        { Result.Add(R); Seen.Add(R.Ability); }
    Result.Sort([](const auto& A, const auto& B) { return uint8(A.Ability) < uint8(B.Ability); });
    return Result;
}
int32 TripoReward::Draw(const TArray<FTripoRewardOption>& Pool, int32 Seed, FName Challenge)
{
    double Sum = 0; for (const auto& R : Pool) Sum += R.Weight;
    if (Pool.IsEmpty() || !FMath::IsFinite(Sum) || Sum <= 0) return INDEX_NONE;
    FRandomStream Random(HashCombineFast(uint32(Seed), FCrc::StrCrc32(*Challenge.ToString())));
    double Pick = Random.FRand() * Sum;
    for (int32 I = 0; I < Pool.Num(); ++I) { Pick -= Pool[I].Weight; if (Pick < 0) return I; }
    return Pool.Num() - 1;
}
