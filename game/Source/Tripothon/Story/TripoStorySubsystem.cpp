#include "Story/TripoStorySubsystem.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Player/TripoCharacter.h"
#include "World/TripoInteractorComponent.h"
UTripoStorySubsystem* UTripoStorySubsystem::Get(const UObject* Context) { return Context && Context->GetWorld() ? Context->GetWorld()->GetSubsystem<UTripoStorySubsystem>() : nullptr; }
bool UTripoStorySubsystem::OpenEvent(ATripoCharacter* Player, UTripoStoryCatalog* Catalog, FName Id)
{
    if (!IsValid(Player) || Player->Interactor->bSuppressed || bOpen || !IsValid(Catalog) || !Catalog->IsValidCatalog()) return false;
    auto* P = UTripoProgressSubsystem::Get(Player);
    if (P->GetPhase() == ETripoChallengePhase::Running || P->GetPhase() == ETripoChallengePhase::PendingReward) return false;
    const auto* E = Catalog->Find(Id); if (!E) return false;
    for (FName Required : E->RequiredEvents) if (!P->HasApplied(Required)) return false;
    for (FName Required : E->RequiredChallenges) if (!P->HasCompleted(Required)) return false;
    if (!P->ApplyStory(Player, Id, E->GrantFloors)) return false;
    Current = *E; Reader = Player; bOpen = true; return true;
}
void UTripoStorySubsystem::CloseEvent(bool bSkip)
{
    if (!bOpen) return; bOpen = false;
    if (Reader.IsValid())
    {
        auto* P = UTripoProgressSubsystem::Get(Reader.Get());
        if (!bSkip) P->MarkViewed(Current.Id);
        if (!Current.NextMap.IsNone()) P->Travel(Reader.Get(), Current.NextMap);
    }
    Reader.Reset();
}
