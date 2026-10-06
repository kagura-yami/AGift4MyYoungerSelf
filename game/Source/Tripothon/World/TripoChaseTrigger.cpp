#include "World/TripoChaseTrigger.h"
#include "World/TripoChaser.h"
#include "World/TripoChaseHideZone.h"
#include "World/TripoInteractorComponent.h"
#include "Player/TripoCharacter.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

void ATripoChaseTrigger::BuildNavigationForLevel()
{
#if WITH_EDITOR
    if (GetWorld() && !GetWorld()->IsGameWorld())
        if (auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld())) Nav->Build();
#endif
}

ATripoChaseTrigger::ATripoChaseTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = .05f;
    Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
    SetRootComponent(Volume);
    Volume->SetBoxExtent(FVector(100,200,120));
    Volume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Volume->SetCanEverAffectNavigation(false);
    Volume->ShapeColor = FColor::Red;
    SpawnMarker = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnMarker"));
    SpawnMarker->SetupAttachment(Volume);
    SpawnMarker->SetRelativeLocation(FVector(-600,0,0));
    ChaserClass = ATripoChaser::StaticClass();
}
bool ATripoChaseTrigger::ActivateChase(ATripoCharacter* Player)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!HasAuthority() || !bEnabled || bUsed || IsValid(ActiveChaser) || !ChaserClass ||
        !IsValid(Player) || !Player->IsPlayerControlled() || Player->Interactor->bSuppressed ||
        (R && (R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running))) return false;
    const FTransform Transform = IsValid(SpawnPoint) ? SpawnPoint->GetActorTransform() : SpawnMarker->GetComponentTransform();
    for (const auto& Zone : ExitZones)
        if (IsValid(Zone) && Zone->ContainsPoint(Player->GetActorLocation())) return false;
    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
    ActiveChaser = GetWorld()->SpawnActor<ATripoChaser>(ChaserClass,Transform.GetLocation(),Transform.Rotator(),Params);
    if (!IsValid(ActiveChaser))
    {
        LastError = TEXT("Chaser spawn blocked: move SpawnPoint above a clear navigable floor.");
        UE_LOG(LogTemp,Warning,TEXT("%s: %s"),*GetName(),*LastError);
        return false;
    }
    if (!ActiveChaser->StartChase(Player))
    {
        ActiveChaser->Destroy(); ActiveChaser = nullptr;
        LastError = TEXT("Chaser requires an AIController and a controlled TripoCharacter.");
        return false;
    }
    bUsed = true;
    ActivationEpoch = R ? R->GetEpoch() : 0;
    LastError.Empty();
    OnChaseStarted.Broadcast(ActiveChaser);
    return true;
}
void ATripoChaseTrigger::EndChase(bool bAllowRetrigger)
{
    if (IsValid(ActiveChaser)) { ActiveChaser->StopChase(); ActiveChaser->Destroy(); }
    ActiveChaser = nullptr;
    bUsed = !bAllowRetrigger;
    // Require leaving the trigger before a new encounter, even when reset happens inside it.
    bWasInside = true;
}
void ATripoChaseTrigger::EndPlay(const EEndPlayReason::Type Reason)
{
    EndChase(false);
    Super::EndPlay(Reason);
}
void ATripoChaseTrigger::Tick(float Dt)
{
    Super::Tick(Dt);
    if (!HasAuthority()) return;
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (R && bUsed && R->GetEpoch()!=ActivationEpoch)
    {
        EndChase(bRearmAfterRestore);
        ActivationEpoch = R->GetEpoch();
    }
    if (R && (R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running)) return;
    auto* Player = Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if (!IsValid(Player)) return;
    for (const auto& Zone : ExitZones)
        if (IsValid(Zone) && Zone->ContainsPoint(Player->GetActorLocation()))
        {
            if (IsValid(ActiveChaser)) EndChase(false);
            return;
        }
    const FVector Local = Volume->GetComponentTransform().InverseTransformPosition(Player->GetActorLocation());
    const FVector Extent = Volume->GetUnscaledBoxExtent();
    const bool bInside = FMath::Abs(Local.X)<=Extent.X && FMath::Abs(Local.Y)<=Extent.Y && FMath::Abs(Local.Z)<=Extent.Z;
    if (bInside && !bWasInside) ActivateChase(Player);
    bWasInside = bInside;
}
