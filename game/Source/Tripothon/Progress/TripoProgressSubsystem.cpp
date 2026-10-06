#include "Progress/TripoProgressSubsystem.h"
#include "Progress/TripoRewardRules.h"
#include "Player/TripoCharacter.h"
#include "Abilities/TripoAbilityComponent.h"
#include "World/TripoWorldSubsystem.h"
#include "World/TripoInteractorComponent.h"
#include "World/TripoZone.h"
#include "EngineUtils.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
void UTripoProgressSubsystem::Initialize(FSubsystemCollectionBase& Collection) { Super::Initialize(Collection); RunSeed = int32(GetTypeHash(FGuid::NewGuid())); }
UTripoProgressSubsystem* UTripoProgressSubsystem::Get(const UObject* Context)
{ auto* GI = UGameplayStatics::GetGameInstance(Context); return GI ? GI->GetSubsystem<UTripoProgressSubsystem>() : nullptr; }
bool UTripoProgressSubsystem::Start(ATripoCharacter* Player, UTripoChallengeDefinition* Definition)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(Player);
    if (!IsValid(Player) || !R || R->IsActionPaused() || Player->Interactor->bSuppressed || !IsValid(Definition) || !Definition->IsValidDefinition() || Phase == ETripoChallengePhase::Running || Phase == ETripoChallengePhase::PendingReward) return false;
    for (uint8 I = 0; I < 8; ++I) if (!Player->Abilities->MeetsLevelFloor(ETripoAbility(I), Definition->RequiredLevels[I])) { Message = TEXT("Required ability level missing"); return false; }
    if (!UTripoWorldSubsystem::Get(Player)->SetCheckpoint(Player, Player->GetActorTransform(), true)) { Message = TEXT("Start requires a stable safe floor"); return false; }
    Current = DuplicateObject<UTripoChallengeDefinition>(Definition, this);
    CurrentId = Current->ChallengeId; StartedAt = R->GetActionSeconds();
    EffectiveBudget = Current->Budget + Player->Abilities->GetBonusTimeSeconds();
    FinishElapsed = 0; bChoice = true; bPractice = Completed.Contains(CurrentId);
    Candidates.Empty(); RandomIndex = INDEX_NONE; Phase = ETripoChallengePhase::Running;
    Message = bPractice ? TEXT("Practice: no additional reward") : TEXT("Challenge started"); return true;
}
double UTripoProgressSubsystem::GetElapsed() const
{
    auto* R = GetGameInstance()->GetSubsystem<UTripoRuntimeSubsystem>();
    return Phase == ETripoChallengePhase::Running && R ? FMath::Max(0., R->GetActionSeconds() - StartedAt) : FinishElapsed;
}
bool UTripoProgressSubsystem::Finish(ATripoCharacter* Player, FName Id)
{
    if (!IsValid(Player) || Player->Interactor->bSuppressed || Phase != ETripoChallengePhase::Running || Id != CurrentId) return false;
    FinishElapsed = GetElapsed(); bChoice = TripoReward::ChoiceEligible(FinishElapsed, EffectiveBudget);
    Candidates = bPractice ? TArray<FTripoRewardOption>() : TripoReward::Filter(Current->Rewards, Player->Abilities->ExportLevels());
    RandomIndex = TripoReward::Draw(Candidates, RunSeed, CurrentId);
    Phase = ETripoChallengePhase::PendingReward;
    Message = Candidates.IsEmpty() ? TEXT("Completed: commemorative record (no available upgrade)") : bChoice ? TEXT("Choose one available ability upgrade") : TEXT("Time exceeded: confirm the fixed random upgrade");
    return true;
}
bool UTripoProgressSubsystem::CommitReward(ATripoCharacter* Player, int32 ChoiceIndex)
{
    if (!IsValid(Player) || Phase != ETripoChallengePhase::PendingReward) return false;
    if (!bPractice && Completed.Contains(CurrentId)) return false;
    const int32 Index = bChoice ? ChoiceIndex : RandomIndex;
    if (!Candidates.IsEmpty() && !Candidates.IsValidIndex(Index)) return false;
    if (!bPractice && Candidates.IsValidIndex(Index))
    {
        const auto Id = Candidates[Index].Ability;
        const int32 Level = Player->Abilities->GetLevel(Id);
        if (Level < 3 && !Player->Abilities->GrantLevelFloor(Id, Level + 1)) return false;
        Message = Level < 3 ? TEXT("Upgrade committed") : TEXT("Candidate no longer available: recorded completion without reroll");
    }
    Completed.Add(CurrentId); Phase = ETripoChallengePhase::Committed;
    SaveSafe(Player);
    return true;
}
bool UTripoProgressSubsystem::Restart(ATripoCharacter* Player)
{
    if (Phase != ETripoChallengePhase::Running || !UTripoWorldSubsystem::Get(Player)->RestorePlayer(Player, true)) return false;
    StartedAt = UTripoRuntimeSubsystem::GetRuntime(Player)->GetActionSeconds();
    FinishElapsed = 0; return true;
}
bool UTripoProgressSubsystem::Exchange(ATripoCharacter* Player, FGuid Transaction, ETripoAbility From, ETripoAbility To, const TArray<int32>& Floors)
{
    Message = TEXT("交换未完成：请在邮差安全区选择不同能力，保留基础等级，并等待相关技能结束。挑战中不能交换。");
    if (!IsValid(Player) || !Transaction.IsValid() || Exchanges.Contains(Transaction) || Phase == ETripoChallengePhase::Running || Phase == ETripoChallengePhase::PendingReward || Player->Interactor->bSuppressed || From == To || uint8(From) >= 8 || uint8(To) >= 8 || Floors.Num() != 8) return false;
    for (int32 Floor : Floors) if (Floor < 0 || Floor > 3) return false;
    if (!ATripoZone::Inside(Player->GetWorld(), ETripoZoneKind::NPC, Player->GetActorLocation())) return false;
    if (Player->Abilities->HasActiveAbility(From) || Player->Abilities->HasActiveAbility(To)) return false;
    auto Levels = Player->Abilities->ExportLevels();
    int32 Protected = Floors[uint8(From)];
    for (TActorIterator<ATripoZone> It(Player->GetWorld()); It; ++It)
        if (It->Kind == ETripoZoneKind::NPC && It->Contains(Player->GetActorLocation()) && It->ProtectedLevels.Num() == 8) Protected = FMath::Max(Protected, It->ProtectedLevels[uint8(From)]);
    if (Levels[uint8(From)] <= Protected || Levels[uint8(To)] >= 3) return false;
    --Levels[uint8(From)]; ++Levels[uint8(To)];
    if (!Player->Abilities->ImportLevels(Levels)) return false;
    Exchanges.Add(Transaction); Message = TEXT("Exchange committed"); return true;
}

int32 UTripoProgressSubsystem::GetFinishGiftCount(FName Id) const
{
    const FString Key=TEXT("ChallengeGifts.")+Id.ToString();
    return Viewed.Contains(FName(*(Key+TEXT(".Two")))) ? 2 : Viewed.Contains(FName(*(Key+TEXT(".One")))) ? 1 : 0;
}
bool UTripoProgressSubsystem::FinishWithGifts(ATripoCharacter* Player, FName Id)
{
    auto* R=UTripoRuntimeSubsystem::GetRuntime(Player);
    if (!IsValid(Player) || !Player->IsPlayerControlled() || !R || R->IsActionPaused() || Player->Interactor->bSuppressed ||
        Phase!=ETripoChallengePhase::Running || CurrentId!=Id || Completed.Contains(Id)) return false;
    if (!UTripoWorldSubsystem::Get(Player)->SetCheckpoint(Player,Player->GetActorTransform())) return false;
    FinishElapsed=GetElapsed(); bChoice=TripoReward::ChoiceEligible(FinishElapsed,EffectiveBudget);
    Viewed.Add(FName(*(TEXT("ChallengeGifts.")+Id.ToString()+(bChoice ? TEXT(".Two") : TEXT(".One")))));
    Completed.Add(Id); Candidates.Empty(); Phase=ETripoChallengePhase::Committed;
    SaveSafe(Player);
    return true;
}
