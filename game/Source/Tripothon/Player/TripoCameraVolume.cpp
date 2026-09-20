#include "Player/TripoCameraVolume.h"
#include "Components/BoxComponent.h"
#include "Core/TripoIdentityComponent.h"

ATripoCameraVolume::ATripoCameraVolume()
{
    PrimaryActorTick.bCanEverTick = false;
    Identity = CreateDefaultSubobject<UTripoIdentityComponent>(TEXT("Identity"));
    Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
    RootComponent = Bounds;
    Bounds->SetBoxExtent(FVector(250.f, 250.f, 300.f));
    Bounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Bounds->SetCollisionResponseToAllChannels(ECR_Ignore);
    Bounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Bounds->SetGenerateOverlapEvents(true);
}
