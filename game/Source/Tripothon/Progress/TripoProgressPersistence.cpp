#include "Progress/TripoProgressSubsystem.h"
#include "Progress/TripoSaveGame.h"
#include "Player/TripoCharacter.h"
#include "Abilities/TripoAbilityComponent.h"
#include "World/TripoWorldSubsystem.h"
#include "World/TripoZone.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Core/TripoIdentityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Misc/PackageName.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UTripoProgressSubsystem::IsLabWorld(const UObject* Context) const
{ return Context && Context->GetWorld() && Context->GetWorld()->GetMapName().Contains(TEXT("L_LogicLab")); }
bool UTripoProgressSubsystem::SaveSafe(ATripoCharacter* Player)
{
    if (IsValid(Player) && !Player->GetCharacterMovement()->IsMovingOnGround()) { Message = TEXT("请落到安全地面后保存"); return false; }
    if (!IsValid(Player) || Phase == ETripoChallengePhase::Running || Phase == ETripoChallengePhase::PendingReward || !UTripoWorldSubsystem::Get(Player)->IsSafeSpawn(Player, Player->GetActorLocation())) { Message = TEXT("Save unavailable: finish challenge or reach safe ground"); return false; }
    if (Phase != ETripoChallengePhase::Committed && !ATripoZone::Inside(Player->GetWorld(), ETripoZoneKind::Safe, Player->GetActorLocation()) && !ATripoZone::Inside(Player->GetWorld(), ETripoZoneKind::NPC, Player->GetActorLocation())) { Message = TEXT("Save at a marked safe area"); return false; }
    auto* Save = NewObject<UTripoSaveGame>(); Save->RunId = UTripoRuntimeSubsystem::GetRuntime(Player)->GetRunId();
    Save->Sequence = SaveSequence + 1; Save->RunSeed = RunSeed; Save->Levels = Player->Abilities->ExportLevels();
    Save->Completed = Completed; Save->Viewed = Viewed; Save->Applied = Applied; Save->Exchanges = Exchanges;
    Save->ReplyChoice = ReplyChoice;
    Save->bLab = IsLabWorld(Player); Save->Spawn = Player->GetActorTransform();
    Save->MapPackage = Player->GetWorld()->GetOutermost()->GetName();
    // Remove only the PIE leaf prefix, preserving the package directory.
    const FString Leaf = FPackageName::GetShortName(Save->MapPackage);
    if (Leaf.StartsWith(TEXT("UEDPIE_"))) { int32 End = Leaf.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromStart, 7); if (End != INDEX_NONE) Save->MapPackage = FPackageName::GetLongPackagePath(Save->MapPackage) / Leaf.Mid(End+1); }
    for (TActorIterator<ATripoMechanism> It(Player->GetWorld()); It; ++It) Save->Mechanisms.Add(It->Identity->GetStableId(), It->Capture());
    if (!Save->IsValidData()) { Message = TEXT("Save rejected: invalid data"); return false; }
    const FString Slot = FString::Printf(TEXT("Tripothon%s_%c"), Save->bLab ? TEXT("Lab") : TEXT("Story"), Save->Sequence % 2 ? TCHAR('A') : TCHAR('B'));
    if (!UGameplayStatics::SaveGameToSlot(Save, Slot, 0)) { Message = TEXT("Save failed; previous slot retained"); return false; }
    auto* Verify = Cast<UTripoSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot,0));
    if (!Verify || !Verify->IsValidData() || Verify->Sequence != Save->Sequence || Verify->RunId != Save->RunId) { Message = TEXT("Save verification failed; previous slot retained"); return false; }
    SaveSequence = Save->Sequence; Message = TEXT("Saved at safe point"); return true;
}
bool UTripoProgressSubsystem::ContinueGame(bool bLab)
{
    UTripoSaveGame* Best = nullptr;
    for (const TCHAR* Suffix : {TEXT("A"),TEXT("B")})
    {
        const FString Slot = FString::Printf(TEXT("Tripothon%s_%s"), bLab ? TEXT("Lab") : TEXT("Story"), Suffix);
        if (!UGameplayStatics::DoesSaveGameExist(Slot,0)) continue;
        auto* Save = Cast<UTripoSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot,0));
        if (Save && Save->IsValidData() && Save->bLab == bLab && FPackageName::DoesPackageExist(Save->MapPackage) && (!Best || Save->Sequence > Best->Sequence)) Best = Save;
    }
    if (!Best) { Message = TEXT("No compatible safe save; start a new game"); return false; }
    PendingTravelLevels.Empty(); PendingLoad = Best; Completed = Best->Completed; Viewed = Best->Viewed; Applied = Best->Applied; Exchanges = Best->Exchanges;
    ReplyChoice = Best->ReplyChoice;
    RunSeed = Best->RunSeed; SaveSequence = Best->Sequence; Phase = ETripoChallengePhase::Idle; Candidates.Empty(); Current = nullptr;
    GetGameInstance()->GetSubsystem<UTripoRuntimeSubsystem>()->BeginRun(Best->RunId);
    GetGameInstance()->GetSubsystem<UTripoRuntimeSubsystem>()->SetPauseReason(ETripoPauseReason::Menu, false);
    UGameplayStatics::OpenLevel(GetGameInstance(), FName(*Best->MapPackage)); return true;
}
bool UTripoProgressSubsystem::ApplyPendingLoad(ATripoCharacter* Player)
{
    if (IsValid(Player) && !PendingTravelLevels.IsEmpty())
    {
        Player->Abilities->ImportLevels(PendingTravelLevels); PendingTravelLevels.Empty();
        UTripoWorldSubsystem::Get(Player)->SetCheckpoint(Player, Player->GetActorTransform());
        Message = TEXT("已进入下一章节"); return true;
    }
    if (!PendingLoad || !IsValid(Player)) return false;
    Player->Abilities->ImportLevels(PendingLoad->Levels);
    for (TActorIterator<ATripoMechanism> It(Player->GetWorld()); It; ++It)
        if (const auto* State = PendingLoad->Mechanisms.Find(It->Identity->GetStableId())) It->Restore(*State);
    auto* World = UTripoWorldSubsystem::Get(Player);
    if (World->IsSafeSpawn(Player, PendingLoad->Spawn.GetLocation()))
    {
        Player->TeleportTo(PendingLoad->Spawn.GetLocation(), PendingLoad->Spawn.Rotator(), false, true);
        Player->ResetAfterRestore(); World->SetCheckpoint(Player, Player->GetActorTransform());
        Message = TEXT("Continued from safe save; unfinished challenge restarts");
    }
    else Message = TEXT("Saved position blocked; using level start with saved progress");
    PendingLoad = nullptr; return true;
}
void UTripoProgressSubsystem::NewGame(bool bLab)
{
    PendingLoad = nullptr; Completed.Empty(); Viewed.Empty(); Applied.Empty(); Exchanges.Empty(); Candidates.Empty(); Current = nullptr;
    ReplyChoice = INDEX_NONE; PendingTravelLevels.Empty();
    Phase = ETripoChallengePhase::Idle; CurrentId = NAME_None; RunSeed = int32(GetTypeHash(FGuid::NewGuid()));
    // Keep a monotonically newer generation than both old slots, so a new run's first save wins.
    for (const TCHAR* Suffix : {TEXT("A"),TEXT("B")})
    {
        const FString Slot = FString::Printf(TEXT("Tripothon%s_%s"), bLab ? TEXT("Lab") : TEXT("Story"), Suffix);
        if (!UGameplayStatics::DoesSaveGameExist(Slot, 0)) continue;
        auto* Old = Cast<UTripoSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot,0));
        if (Old && Old->IsValidData()) SaveSequence = FMath::Max(SaveSequence, Old->Sequence);
    }
    GetGameInstance()->GetSubsystem<UTripoRuntimeSubsystem>()->BeginRun(FGuid::NewGuid());
    GetGameInstance()->GetSubsystem<UTripoRuntimeSubsystem>()->SetPauseReason(ETripoPauseReason::Menu, false);
    UGameplayStatics::OpenLevel(GetGameInstance(), bLab ? TEXT("/Game/Maps/L_LogicLab") : TEXT("/Game/Maps/L_Tutorial"));
}
bool UTripoProgressSubsystem::Travel(ATripoCharacter* Player, FName Map)
{
    if (!IsValid(Player) || !Map.ToString().StartsWith(TEXT("/Game/Maps/")) || !FPackageName::DoesPackageExist(Map.ToString()) || !SaveSafe(Player)) return false;
    PendingLoad = nullptr; PendingTravelLevels = Player->Abilities->ExportLevels();
    Player->Abilities->CancelAll(); UGameplayStatics::OpenLevel(Player, Map); return true;
}
bool UTripoProgressSubsystem::ApplyStory(ATripoCharacter* Player, FName Id, const TArray<int32>& Floors)
{
    if (!IsValid(Player) || Id.IsNone() || Floors.Num() != 8) return false;
    for (int32 V : Floors) if (V < 0 || V > 3) return false;
    // Reapply floor even if effects were already committed; never add one for a story grant.
    for (uint8 I = 0; I < 8; ++I) Player->Abilities->GrantLevelFloor(ETripoAbility(I), Floors[I]);
    Applied.Add(Id); return true;
}
bool UTripoProgressSubsystem::SelectReply(int32 Choice)
{
    if (Choice < 0 || Choice > 2 || !HasApplied(TEXT("Story.Finale.SendReply"))) return false;
    if (ReplyChoice != INDEX_NONE) return ReplyChoice == Choice;
    ReplyChoice = Choice;
    return true;
}
