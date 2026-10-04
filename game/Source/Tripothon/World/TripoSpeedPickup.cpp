#include "World/TripoSpeedPickup.h"
#include "Components/SphereComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Kismet/GameplayStatics.h"
ATripoSpeedPickup::ATripoSpeedPickup()
{
    PrimaryActorTick.bCanEverTick = true;
    Volume = CreateDefaultSubobject<USphereComponent>(TEXT("PickupVolume"));
    SetRootComponent(Volume);
    Volume->InitSphereRadius(55.f);
    Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Volume->SetCollisionResponseToAllChannels(ECR_Ignore);
    Volume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Volume->SetGenerateOverlapEvents(true);
}
void ATripoSpeedPickup::BeginPlay()
{
    Super::BeginPlay();
    if (VisualActor) { VisualActor->SetActorEnableCollision(false); VisualActor->SetActorTickEnabled(false); }
    if (auto* R = UTripoRuntimeSubsystem::GetRuntime(this)) RestoreEpoch = R->GetEpoch();
}
void ATripoSpeedPickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!HasAuthority() || !R) return;
    if (RestoreEpoch != R->GetEpoch())
    {
        RestoreEpoch = R->GetEpoch(); bCollected = false;
        if (VisualActor) VisualActor->SetActorHiddenInGame(false);
    }
    if (bCollected || R->IsActionPaused() || R->GetRestorePhase() != ETripoRestorePhase::Running) return;
    auto* Player = Cast<ATripoCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (Player && Volume->IsOverlappingActor(Player) && Player->ApplySpeedBuff(Multiplier, Duration))
    {
        bCollected = true;
        if (VisualActor) VisualActor->SetActorHiddenInGame(true);
    }
}
