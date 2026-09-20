#include "Abilities/TripoAbilityComponent.h"
#include "Abilities/TripoAbilityInstance.h"
#include "Core/TripoRuntimeSubsystem.h"

UTripoAbilityComponent::UTripoAbilityComponent() { PrimaryComponentTick.bCanEverTick = true; }
TArray<int32> UTripoAbilityComponent::ExportLevels() const
{
    TArray<int32> Result; for (uint8 I = 0; I < 8; ++I) Result.Add(GetLevel(static_cast<ETripoAbility>(I))); return Result;
}
bool UTripoAbilityComponent::ImportLevels(const TArray<int32>& Values)
{
    if (!bInitialized || Values.Num() != 8 || bMutating) return false;
    for (int32 V : Values) if (V < 0 || V > 3) return false;
    for (uint8 I = 0; I < 8; ++I) Levels.Add(static_cast<ETripoAbility>(I), Values[I]);
    return true;
}
TArray<double> UTripoAbilityComponent::ExportCooldowns() const
{
    TArray<double> Result; for (uint8 I = 0; I < 8; ++I) Result.Add(GetCooldownRemaining(static_cast<ETripoAbility>(I))); return Result;
}
bool UTripoAbilityComponent::ImportCooldowns(const TArray<double>& Values)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || Values.Num() != 8 || bMutating) return false;
    for (double V : Values) if (!FMath::IsFinite(V) || V < 0) return false;
    const double Now = R->GetActionSeconds();
    for (uint8 I = 0; I < 8; ++I) ReadyAt.Add(static_cast<ETripoAbility>(I), Now + Values[I]);
    return true;
}
bool UTripoAbilityComponent::HasActiveAbility(ETripoAbility Id) const
{
    const auto* I = Instances.FindRef(Id).Get(); return I && I->IsActive();
}
bool UTripoAbilityComponent::GetParameters(ETripoAbility Id, FTripoAbilityParameters& Parameters) const
{
    const auto* D = Catalog.FindRef(Id).Get(); const auto* P = D ? D->Parameters(GetLevel(Id)) : nullptr;
    if (!P) return false; Parameters = *P; return true;
}
void UTripoAbilityComponent::BeginPlay() { Super::BeginPlay(); InitializeDefinitions(); }
void UTripoAbilityComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    bEndingPlay = true;
    CancelAll();
    Super::EndPlay(Reason);
}
bool UTripoAbilityComponent::InitializeDefinitions()
{
    if (bInitialized) return true;
    TMap<ETripoAbility, TObjectPtr<UTripoAbilityDefinition>> Candidate;
    for (const auto& D : Definitions)
    {
        if (!IsValid(D) || !D->IsValidDefinition() || Candidate.Contains(D->Ability)) return false;
        Candidate.Add(D->Ability, D);
    }
    for (uint8 Id = 0; Id <= static_cast<uint8>(ETripoAbility::BonusTime); ++Id)
    {
        const auto Ability = static_cast<ETripoAbility>(Id);
        if (!Candidate.Contains(Ability)) Candidate.Add(Ability, UTripoAbilityDefinition::MakeDefaults(this, Ability));
    }
    Catalog = MoveTemp(Candidate);
    for (const auto& Pair : Catalog)
    {
        Levels.Add(Pair.Key, 0);
        if (Pair.Value->Implementation && !Pair.Value->Implementation->HasAnyClassFlags(CLASS_Abstract))
            Instances.Add(Pair.Key, NewObject<UTripoAbilityInstance>(this, Pair.Value->Implementation));
    }
    bInitialized = true;
    return true;
}
int32 UTripoAbilityComponent::GetLevel(ETripoAbility Id) const { return Levels.FindRef(Id); }
bool UTripoAbilityComponent::GrantLevelFloor(ETripoAbility Id, int32 Minimum)
{
    if (!bInitialized || !Catalog.Contains(Id) || Minimum < 0 || Minimum > 3) return false;
    Levels.FindOrAdd(Id) = FMath::Max(GetLevel(Id), Minimum);
    return true;
}
bool UTripoAbilityComponent::MeetsLevelFloor(ETripoAbility Id, int32 Required) const
{
    return bInitialized && Catalog.Contains(Id) && Required >= 0 && Required <= 3 && GetLevel(Id) >= Required;
}
double UTripoAbilityComponent::GetCooldownRemaining(ETripoAbility Id) const
{
    if (!GetWorld()) return 0.;
    auto* Runtime = UTripoRuntimeSubsystem::GetRuntime(this);
    return Runtime ? FMath::Max(0., ReadyAt.FindRef(Id) - Runtime->GetActionSeconds()) : 0.;
}
float UTripoAbilityComponent::GetBonusTimeSeconds() const
{
    const auto* D = Catalog.FindRef(ETripoAbility::BonusTime).Get();
    const auto* P = D ? D->Parameters(GetLevel(ETripoAbility::BonusTime)) : nullptr;
    return P ? P->Strength : 0.f;
}
ETripoAbilityFailure UTripoAbilityComponent::TryActivate(ETripoAbility Id, AActor* Target, FGuid& OutHandle)
{
    OutHandle.Invalidate();
    // Guard callbacks from recursively activating or cancelling halfway through a transaction.
    if (bMutating || bEndingPlay) return ETripoAbilityFailure::InvalidContext;
    TGuardValue<bool> Guard(bMutating, true);
    auto Fail = [&](ETripoAbilityFailure Why) { ReportFailure(Id, Why); return Why; };
    const auto* D = Catalog.FindRef(Id).Get();
    if (!bInitialized || !D || !D->IsValidDefinition()) return Fail(ETripoAbilityFailure::InvalidDefinition);
    const auto* P = D->Parameters(GetLevel(Id));
    if (!P) return Fail(ETripoAbilityFailure::Locked);
    if (Id == ETripoAbility::BonusTime) return Fail(ETripoAbilityFailure::Passive);
    if (D->bRequiresTarget && !IsValid(Target)) return Fail(ETripoAbilityFailure::NoTarget);
    auto* Runtime = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!Runtime || !GetOwner()) return Fail(ETripoAbilityFailure::InvalidContext);
    if (Runtime->IsActionPaused()) return Fail(ETripoAbilityFailure::Paused);
    if (Runtime->GetRestorePhase() != ETripoRestorePhase::Running) return Fail(ETripoAbilityFailure::Restoring);
    auto* Instance = Instances.FindRef(Id).Get();
    if (!Instance) return Fail(ETripoAbilityFailure::NotImplemented);
    if (Instance->IsActive()) return Fail(ETripoAbilityFailure::AlreadyActive);
    const double Now = Runtime->GetActionSeconds();
    if (ReadyAt.FindRef(Id) > Now) return Fail(ETripoAbilityFailure::Cooldown);
    auto Result = Instance->Validate(Target, *P);
    if (Result != ETripoAbilityFailure::None) return Fail(Result);
    Result = Instance->BeginEffect(Target, *P);
    if (Result != ETripoAbilityFailure::None) return Fail(Result);
    Instance->Handle = FGuid::NewGuid();
    Instance->bCompletionRequested = false;
    Instance->LastUpdate = Now;
    const double Duration = Instance->GetExecutionDuration(*P);
    Instance->EndsAt = Now + Duration;
    ReadyAt.Add(Id, Now + P->Cooldown);
    OutHandle = Instance->Handle;
    LastFailure = ETripoAbilityFailure::None;
    OnStarted.Broadcast(Id, OutHandle, ETripoAbilityFailure::None, false);
    if (Duration == 0) Finish(Id, Instance, false);
    return ETripoAbilityFailure::None;
}
void UTripoAbilityComponent::Finish(ETripoAbility Id, UTripoAbilityInstance* Instance, bool bCancelled)
{
    const FGuid Old = Instance->Handle;
    Instance->Handle.Invalidate();
    Instance->EndEffect(bCancelled);
    OnEnded.Broadcast(Id, Old, ETripoAbilityFailure::None, bCancelled);
}
bool UTripoAbilityComponent::ReportHit(FGuid Handle, const FHitResult& Hit)
{
    if (!Handle.IsValid()) return false;
    TGuardValue<bool> Guard(bMutating, true);
    for (const auto& Pair : Instances)
        if (Pair.Value->Handle == Handle)
        {
            OnHit.Broadcast(Pair.Key, Handle, Hit);
            return true;
        }
    return false;
}
bool UTripoAbilityComponent::Cancel(FGuid Handle)
{
    if (bMutating || !Handle.IsValid()) return false;
    TGuardValue<bool> Guard(bMutating, true);
    for (const auto& Pair : Instances)
        if (Pair.Value->Handle == Handle) { Finish(Pair.Key, Pair.Value, true); return true; }
    return false;
}
bool UTripoAbilityComponent::CancelAbility(ETripoAbility Id)
{ const auto* I = Instances.FindRef(Id).Get(); return I && Cancel(I->GetHandle()); }
void UTripoAbilityComponent::ReportFailure(ETripoAbility Id, ETripoAbilityFailure Failure)
{ LastFailure = Failure; OnFailed.Broadcast(Id, FGuid(), Failure, false); }
FString UTripoAbilityComponent::GetFailureMessage() const
{
    switch (LastFailure)
    {
    case ETripoAbilityFailure::None: return TEXT("");
    case ETripoAbilityFailure::Locked: return TEXT("此能力尚未解锁");
    case ETripoAbilityFailure::NoTarget: return TEXT("没有可用目标或历史不足");
    case ETripoAbilityFailure::Cooldown: return TEXT("能力仍在冷却");
    case ETripoAbilityFailure::AlreadyActive: return TEXT("能力正在运行");
    case ETripoAbilityFailure::Capacity: return TEXT("石块已达数量上限，请等待到期");
    case ETripoAbilityFailure::AirUseSpent: return TEXT("本次空中次数已使用，需要重新落地或离开墙面");
    case ETripoAbilityFailure::Blocked: return TEXT("位置被阻挡或当前状态不允许");
    default: return TEXT("当前无法使用此能力");
    }
}
void UTripoAbilityComponent::CancelAll()
{
    if (bMutating) return;
    TGuardValue<bool> Guard(bMutating, true);
    for (const auto& Pair : Instances) if (Pair.Value->IsActive()) Finish(Pair.Key, Pair.Value, true);
}
void UTripoAbilityComponent::TickComponent(float Delta, ELevelTick TickType, FActorComponentTickFunction* Function)
{
    Super::TickComponent(Delta, TickType, Function);
    auto* Runtime = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!Runtime || bMutating) return;
    if (Runtime->GetRestorePhase() != ETripoRestorePhase::Running) { CancelAll(); return; }
    if (Runtime->IsActionPaused()) return;
    TGuardValue<bool> Guard(bMutating, true);
    const double Now = Runtime->GetActionSeconds();
    for (const auto& Pair : Instances)
    {
        auto* I = Pair.Value.Get();
        if (!I->IsActive()) continue;
        const double Sample = FMath::Min(Now, I->EndsAt);
        const double Elapsed = FMath::Max(0., Sample - I->LastUpdate);
        I->LastUpdate = Sample;
        if (Elapsed > 0) I->UpdateEffect(Elapsed);
        if (Now >= I->EndsAt || I->bCompletionRequested) Finish(Pair.Key, I, false);
    }
}

