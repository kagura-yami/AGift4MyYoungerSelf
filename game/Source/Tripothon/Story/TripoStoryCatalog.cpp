#include "Story/TripoStoryCatalog.h"
const FTripoStoryEvent* UTripoStoryCatalog::Find(FName Id) const { return Events.FindByPredicate([Id](const auto& E) { return E.Id == Id; }); }
bool UTripoStoryCatalog::IsValidCatalog() const
{
    TSet<FName> Seen;
    for (const auto& E : Events)
    {
        if (E.Id.IsNone() || Seen.Contains(E.Id) || E.GrantFloors.Num() != 8 || E.Text.IsEmpty()) return false;
        Seen.Add(E.Id); for (int32 V : E.GrantFloors) if (V < 0 || V > 3) return false;
    }
    for (const auto& E : Events) for (FName Required : E.RequiredEvents) if (!Seen.Contains(Required) || Required == E.Id) return false;
    TSet<FName> Visiting, Done;
    TFunction<bool(FName)> Visit = [&](FName Id)
    {
        if (Visiting.Contains(Id)) return false;
        if (Done.Contains(Id)) return true;
        Visiting.Add(Id);
        for (FName Required : Find(Id)->RequiredEvents) if (!Visit(Required)) return false;
        Visiting.Remove(Id); Done.Add(Id); return true;
    };
    for (const auto& E : Events) if (!Visit(E.Id)) return false;
    return !Events.IsEmpty();
}
