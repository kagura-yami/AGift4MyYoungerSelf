#include "World/TripoMechanism.h"
#include "World/TripoInteractorComponent.h"
#include "Time/TripoHistoryComponent.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ATripoMechanism::ATripoMechanism()
{
    PrimaryActorTick.bCanEverTick = true;
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
    SetRootComponent(Body);
    Body->SetMobility(EComponentMobility::Movable);
    Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) Body->SetStaticMesh(Cube.Object);
    Body->SetHiddenInGame(true);
    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual")); Visual->SetupAttachment(Body);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (Cube.Succeeded()) Visual->SetStaticMesh(Cube.Object);
    Sensor = CreateDefaultSubobject<UBoxComponent>(TEXT("Sensor"));
    Sensor->SetupAttachment(Body);
    Sensor->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sensor->SetCollisionResponseToAllChannels(ECR_Ignore);
    Sensor->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Sensor->SetGenerateOverlapEvents(true);
    Sensor->SetBoxExtent(FVector(55, 55, 90));
    Sensor->SetRelativeLocation(FVector(0, 0, 110));
    Identity = CreateDefaultSubobject<UTripoIdentityComponent>(TEXT("Identity"));
    History = CreateDefaultSubobject<UTripoHistoryComponent>(TEXT("History"));
}
void ATripoMechanism::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    Sensor->SetCollisionEnabled(Kind == ETripoMechanismKind::Plate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}
void ATripoMechanism::BeginPlay()
{
    Super::BeginPlay();
    Origin = GetActorLocation();
    if (auto* R = UTripoRuntimeSubsystem::GetRuntime(this)) LastAction = R->GetActionSeconds();
}
void ATripoMechanism::RefreshOccupants()
{
    Occupants.Empty();
    if (Kind != ETripoMechanismKind::Plate) return;
    TArray<AActor*> Actors;
    Sensor->GetOverlappingActors(Actors);
    for (auto* A : Actors)
        if (IsValid(A))
            if (auto* I = A->FindComponentByClass<UTripoInteractorComponent>(); I && I->CanTrigger(bAllowEcho)) Occupants.Add(A);
}
bool ATripoMechanism::Evaluate(TSet<const ATripoMechanism*>& Visiting) const
{
    const bool bProgressCondition = !RequiredEvents.IsEmpty() || !RequiredChallenges.IsEmpty();
    if (bProgressCondition)
    {
        auto* P = UTripoProgressSubsystem::Get(this); if (!P) return false;
        for (FName Id : RequiredEvents) if (!P->HasApplied(Id)) return false;
        for (FName Id : RequiredChallenges) if (!P->HasCompleted(Id)) return false;
    }
    if (Visiting.Contains(this)) return false; // Graph cycles fail closed.
    if (Kind == ETripoMechanismKind::Plate)
    {
        for (const auto& A : Occupants)
            if (A.IsValid())
                if (auto* I = A->FindComponentByClass<UTripoInteractorComponent>(); I && I->CanTrigger(bAllowEcho)) return true;
        return false;
    }
    if (Kind == ETripoMechanismKind::Switch) return bLatched;
    if (Inputs.IsEmpty()) return Kind == ETripoMechanismKind::Platform || bProgressCondition;
    Visiting.Add(this);
    bool Result = bRequireAll;
    for (const auto& Input : Inputs)
    {
        const bool Value = IsValid(Input) && Input->Evaluate(Visiting);
        Result = bRequireAll ? Result && Value : Result || Value;
    }
    Visiting.Remove(this);
    return Result;
}
bool ATripoMechanism::IsPowered() const { TSet<const ATripoMechanism*> Visiting; return Evaluate(Visiting); }
bool ATripoMechanism::Interact(AActor* Source)
{
    if (Kind != ETripoMechanismKind::Switch || !IsValid(Source) || FVector::DistSquared(Source->GetActorLocation(), GetActorLocation()) > FMath::Square(250.)) return false;
    auto* I = Source->FindComponentByClass<UTripoInteractorComponent>();
    if (!I || !I->CanTrigger(bAllowEcho)) return false;
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (R && (R->IsActionPaused() || R->GetRestorePhase() != ETripoRestorePhase::Running)) return false;
    FHitResult Hit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoInteract), false, Source);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Source->GetActorLocation(), GetActorLocation(), ECC_Visibility, Query) && Hit.GetActor() != this) return false;
    bLatched = !bLatched;
    if (auto* H = Source->FindComponentByClass<UTripoHistoryComponent>()) H->RecordInteraction(Identity->GetStableId());
    return true;
}
FTripoMechanismState ATripoMechanism::Capture() const
{
    FTripoMechanismState State; State.Transform = GetActorTransform(); State.bLatched = bLatched; State.Phase = Phase; State.bCollisionEnabled = Body->GetCollisionEnabled() != ECollisionEnabled::NoCollision; return State;
}
void ATripoMechanism::Restore(const FTripoMechanismState& State)
{
    bLatched = State.bLatched; Phase = State.Phase; SlowSources.Empty(); bRewinding = false; Occupants.Empty();
    SetActorTransform(State.Transform, false, nullptr, ETeleportType::TeleportPhysics);
    if (Kind == ETripoMechanismKind::Gate) { Visual->SetVisibility(State.bCollisionEnabled); Body->SetCollisionEnabled(State.bCollisionEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision); }
    if (auto* R = UTripoRuntimeSubsystem::GetRuntime(this)) LastAction = R->GetActionSeconds();
}
FGuid ATripoMechanism::AddSlow(UObject* Source, float Rate, double Duration)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!IsValid(Source) || !bTimeAffectable || Kind != ETripoMechanismKind::Platform || !R || !FMath::IsFinite(Rate) || Rate <= 0 || Rate > 1 || !FMath::IsFinite(Duration) || Duration <= 0) return FGuid();
    const double Now = R->GetActionSeconds();
    const FGuid Id = FGuid::NewGuid(); SlowSources.Add(Id, {Source, Rate, Now, Now + Duration}); return Id;
}
void ATripoMechanism::RemoveSlow(FGuid Handle) { SlowSources.Remove(Handle); }
float ATripoMechanism::GetLocalRate() const
{
    float Rate = 1;
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    const double Now = R ? R->GetActionSeconds() : 0;
    for (const auto& Pair : SlowSources) if (Pair.Value.Source.IsValid() && Pair.Value.EndsAt > Now) Rate = FMath::Min(Rate, Pair.Value.Rate);
    return Rate;
}
double ATripoMechanism::IntegrateLocalTime(double From, double To) const
{
    TArray<double> Boundaries = {From, To};
    for (const auto& Pair : SlowSources)
    {
        if (!Pair.Value.Source.IsValid()) continue;
        for (double T : {Pair.Value.StartsAt, Pair.Value.EndsAt}) if (T > From && T < To) Boundaries.Add(T);
    }
    Boundaries.Sort(); double Result = 0;
    for (int32 I=1; I<Boundaries.Num(); ++I)
    {
        const double Mid = (Boundaries[I-1] + Boundaries[I]) * .5; double Rate = 1;
        for (const auto& Pair : SlowSources)
            if (Pair.Value.Source.IsValid() && Pair.Value.StartsAt <= Mid && Pair.Value.EndsAt > Mid) Rate = FMath::Min(Rate, double(Pair.Value.Rate));
        Result += (Boundaries[I] - Boundaries[I-1]) * Rate;
    }
    return Result;
}
bool ATripoMechanism::ApplyRewindState(const FTripoMechanismState& State)
{
    if (Kind != ETripoMechanismKind::Platform || !bTimeAffectable) return false;
    FHitResult Hit;
    SetActorLocation(State.Transform.GetLocation(), true, &Hit);
    if (Hit.bBlockingHit) return false;
    Phase = State.Phase; return true;
}
void ATripoMechanism::Tick(float Delta)
{
    Super::Tick(Delta);
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R) return;
    const double Now = R->GetActionSeconds();
    const double Step = IntegrateLocalTime(FMath::Min(LastAction, Now), Now); LastAction = Now;
    if (R->IsActionPaused() || R->GetRestorePhase() != ETripoRestorePhase::Running) return;
    for (auto It = SlowSources.CreateIterator(); It; ++It) if (!It.Value().Source.IsValid() || It.Value().EndsAt <= Now) It.RemoveCurrent();
    RefreshOccupants();
    if (Kind == ETripoMechanismKind::Gate)
    {
        const bool bOpen = IsPowered();
        // Opening removes the blocker; closing waits until the volume is clear of pawns.
        if (bOpen) { Body->SetCollisionEnabled(ECollisionEnabled::NoCollision); Visual->SetVisibility(false); }
        else
        {
            FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoGate), false, this);
            if (!GetWorld()->OverlapAnyTestByObjectType(GetActorLocation(), GetActorQuat(), FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeBox(Body->Bounds.BoxExtent), Query))
            { Visual->SetVisibility(true); Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); }
        }
    }
    else if (Kind == ETripoMechanismKind::Platform && IsPowered() && !bRewinding)
    {
        const double NextPhase = FMath::Fmod(Phase + Step / FMath::Max(.1f, Period), 1.);
        const double Alpha = .5 - .5 * FMath::Cos(NextPhase * 2 * PI);
        FHitResult Hit; SetActorLocation(Origin + Travel * Alpha, true, &Hit);
        if (!Hit.bBlockingHit) Phase = NextPhase;
    }
}
void ATripoMechanism::RefreshGate()
{
    if (Kind != ETripoMechanismKind::Gate) return;
    if (IsPowered()) { Visual->SetVisibility(false); Body->SetCollisionEnabled(ECollisionEnabled::NoCollision); return; }
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoGateRefresh), false, this);
    if (!GetWorld()->OverlapAnyTestByObjectType(GetActorLocation(), GetActorQuat(), FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeBox(Body->Bounds.BoxExtent), Query))
    { Visual->SetVisibility(true); Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); }
}
