#include "Story/TripoStoryTrigger.h"
#include "Story/TripoStoryCatalog.h"
#include "Story/TripoStorySubsystem.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
ATripoStoryTrigger::ATripoStoryTrigger()
{
    Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume")); SetRootComponent(Volume); Volume->SetBoxExtent(FVector(90,150,120));
    Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly); Volume->SetCollisionResponseToAllChannels(ECR_Ignore); Volume->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Hint")); Label->SetupAttachment(Volume); Label->SetRelativeLocation(FVector(0,0,160)); Label->SetWorldSize(26);
}
void ATripoStoryTrigger::BeginPlay() { Super::BeginPlay(); Volume->OnComponentBeginOverlap.AddDynamic(this, &ATripoStoryTrigger::Enter); }
void ATripoStoryTrigger::OnConstruction(const FTransform& T) { Super::OnConstruction(T); Label->SetText(FText::FromString(Hint)); }
bool ATripoStoryTrigger::Interact(ATripoCharacter* Player)
{
    if (!IsValid(Player) || FVector::DistSquared(Player->GetActorLocation(), GetActorLocation()) > FMath::Square(250.)) return false;
    FHitResult Hit; FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoStoryInteraction), false, Player);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Player->GetActorLocation(), GetActorLocation() + FVector(0,0,80), ECC_Visibility, Query) && Hit.GetActor() != this) return false;
    return GetWorld()->GetSubsystem<UTripoStorySubsystem>()->OpenEvent(Player, Catalog, EventId);
}
void ATripoStoryTrigger::Enter(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    auto* Player = Cast<ATripoCharacter>(Other);
    if (Player && bAutomatic && !UTripoProgressSubsystem::Get(Player)->HasApplied(EventId)) Interact(Player);
}
