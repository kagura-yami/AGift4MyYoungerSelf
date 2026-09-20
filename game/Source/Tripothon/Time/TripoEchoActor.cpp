#include "Time/TripoEchoActor.h"
#include "World/TripoInteractorComponent.h"
#include "World/TripoMechanism.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Core/TripoIdentityRegistry.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
ATripoEchoActor::ATripoEchoActor()
{
    PrimaryActorTick.bCanEverTick = true; Tags = {TEXT("TripoTemporary"), TEXT("TripoEcho")};
    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule")); SetRootComponent(Capsule); Capsule->InitCapsuleSize(34,88);
    Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly); Capsule->SetCollisionObjectType(ECC_Pawn);
    Capsule->SetCollisionResponseToAllChannels(ECR_Block); Capsule->SetCollisionResponseToChannel(ECC_Pawn,ECR_Ignore);
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body")); Body->SetupAttachment(Capsule); Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (Sphere.Succeeded()) Body->SetStaticMesh(Sphere.Object); Body->SetRelativeScale3D(FVector(.45,.45,1.3));
    Interactor = CreateDefaultSubobject<UTripoInteractorComponent>(TEXT("Interactor")); Interactor->Kind = ETripoInteractor::Echo;
    Identity = CreateDefaultSubobject<UTripoIdentityComponent>(TEXT("Identity")); Identity->SourceType = TripoTags::SourceEcho;
}
void ATripoEchoActor::BeginPlay()
{
    Super::BeginPlay(); auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || Frames.Num() < 2) { Destroy(); return; }
    StartedAt = R->GetActionSeconds(); Epoch = R->GetEpoch(); Cursor = Frames[0].Time - 1.e-6;
}
void ATripoEchoActor::Tick(float Delta)
{
    Super::Tick(Delta); auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || !IsValid(GetOwner()) || Epoch != R->GetEpoch() || Frames.Num() < 2) { Destroy(); return; }
    if (R->IsActionPaused() || R->GetRestorePhase() != ETripoRestorePhase::Running) return;
    const double To = FMath::Min(Frames.Last().Time, Frames[0].Time + R->GetActionSeconds() - StartedAt);
    TArray<double> Stops;
    for (const auto& F : Frames) if (F.Time > Cursor && F.Time < To) Stops.Add(F.Time);
    for (const auto& E : Events) if (E.Time > Cursor && E.Time < To) Stops.AddUnique(E.Time);
    Stops.Add(To); Stops.Sort();
    for (double Time : Stops)
    {
        FTripoHistoryFrame F; UTripoHistoryComponent::Sample(Frames, Time, F);
        FHitResult Hit; SetActorLocation(F.Transform.GetLocation(), true, &Hit);
        if (Hit.bBlockingHit) { Destroy(); return; }
        SetActorRotation(F.Transform.GetRotation());
        for (const auto& E : Events)
            if (E.Time <= Time && !Played.Contains(E.EventId))
            {
                Played.Add(E.EventId);
                if (auto* M = Cast<ATripoMechanism>(UTripoIdentityRegistry::GetRegistry(this)->Resolve(E.TargetId))) M->Interact(this);
            }
        Cursor = Time;
    }
    if (To >= Frames.Last().Time) Destroy();
}
