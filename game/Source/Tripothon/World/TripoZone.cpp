#include "World/TripoZone.h"
#include "World/TripoWorldSubsystem.h"
#include "World/TripoInteractorComponent.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Core/TripoIdentityComponent.h"
#include "Player/TripoCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
ATripoZone::ATripoZone()
{
    Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume")); SetRootComponent(Volume);
    Volume->SetBoxExtent(FVector(100, 200, 100));
    Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Volume->SetCollisionResponseToAllChannels(ECR_Ignore); Volume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label")); Label->SetupAttachment(Volume);
    Label->SetRelativeLocation(FVector(0,0,180)); Label->SetWorldSize(32);
    Label->SetHiddenInGame(true);
    Identity = CreateDefaultSubobject<UTripoIdentityComponent>(TEXT("Identity"));
    ProtectedLevels.Init(0, 8);
}
void ATripoZone::OnConstruction(const FTransform& Transform) { Super::OnConstruction(Transform); Label->SetText(FText::FromString(Hint)); }
void ATripoZone::BeginPlay()
{
    Super::BeginPlay();
    // Authoring labels are not player-facing UI; also override older map instances.
    Label->SetHiddenInGame(true);
    Volume->OnComponentBeginOverlap.AddDynamic(this, &ATripoZone::Enter);
}
bool ATripoZone::Contains(const FVector& Point) const
{
    const FVector Local = Volume->GetComponentTransform().InverseTransformPosition(Point); const FVector Extent = Volume->GetUnscaledBoxExtent();
    return FMath::Abs(Local.X) <= Extent.X && FMath::Abs(Local.Y) <= Extent.Y && FMath::Abs(Local.Z) <= Extent.Z;
}
bool ATripoZone::Inside(UWorld* World, ETripoZoneKind ZoneKind, const FVector& Point)
{ for (TActorIterator<ATripoZone> It(World); It; ++It) if (It->Kind == ZoneKind && It->Contains(Point)) return true; return false; }
void ATripoZone::Enter(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    auto* Player = Cast<ATripoCharacter>(Other);
    if (!Player || Player->Interactor->bSuppressed) return;
    auto* W = UTripoWorldSubsystem::Get(this); auto* P = UTripoProgressSubsystem::Get(this);
    switch (Kind)
    {
    case ETripoZoneKind::Checkpoint: W->SetCheckpoint(Player, FTransform(GetActorRotation(), GetActorLocation() + SafeOffset)); break;
    case ETripoZoneKind::Hazard: W->RestorePlayer(Player); break;
    case ETripoZoneKind::Start: P->Start(Player, Challenge); break;
    case ETripoZoneKind::Finish: P->Finish(Player, ChallengeId); break;
    default: break;
    }
}
