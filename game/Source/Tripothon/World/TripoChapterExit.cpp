#include "World/TripoChapterExit.h"
#include "Components/BoxComponent.h"
#include "Player/TripoCharacter.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Kismet/GameplayStatics.h"

ATripoChapterExit::ATripoChapterExit()
{
    Volume=CreateDefaultSubobject<UBoxComponent>(TEXT("ExitVolume"));
    SetRootComponent(Volume);
    Volume->SetBoxExtent(FVector(45,65,115));
    Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Volume->SetCollisionResponseToAllChannels(ECR_Ignore);
    Volume->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
    Volume->SetGenerateOverlapEvents(true);
    PrimaryActorTick.bCanEverTick=true;
    PrimaryActorTick.TickInterval=.2f;
}
void ATripoChapterExit::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(!HasAuthority() || bTravelling || Destination.IsNone()) return;
    auto* Player=Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if(!Player || !Volume->IsOverlappingActor(Player)) return;
    auto* Progress=UTripoProgressSubsystem::Get(this);
    if(Progress && !Progress->HasPendingLoad()) bTravelling=Progress->Travel(Player,Destination);
}
