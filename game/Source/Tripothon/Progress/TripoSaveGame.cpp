#include "Progress/TripoSaveGame.h"
bool UTripoSaveGame::IsValidData() const
{
    if (ReplyChoice < INDEX_NONE || ReplyChoice > 2) return false;
    if (SchemaVersion != 1 || Sequence < 0 || !RunId.IsValid() || Levels.Num() != 8 || Spawn.ContainsNaN() || !Spawn.GetScale3D().Equals(FVector::OneVector, .001) || !MapPackage.StartsWith(TEXT("/Game/Maps/")) || MapPackage.Contains(TEXT("..")) || Completed.Num() > 1024 || Viewed.Num() > 1024 || Applied.Num() > 1024 || Mechanisms.Num() > 4096 || Exchanges.Num() > 4096) return false;
    for (int32 Level : Levels) if (Level < 0 || Level > 3) return false;
    if (Gifts.Num() > 4096) return false;
    for (const auto& Gift : Gifts) if (Gift.Key.IsNone() || !Gift.Value.IsValid()) return false;
    for (const auto& Pair : Mechanisms) if (!Pair.Key.IsValid() || Pair.Value.Transform.ContainsNaN() || !FMath::IsFinite(Pair.Value.Phase) || Pair.Value.Phase < 0 || Pair.Value.Phase >= 1) return false;
    if (ElevatorFloors.Num()>4096) return false;
    for (const auto& Pair : ElevatorFloors) if (!Pair.Key.IsValid() || Pair.Value<0 || Pair.Value>1) return false;
    return true;
}
