#include "Story/TripoStoryTrigger.h"
#include "Story/TripoStoryCatalog.h"
#include "Story/TripoStorySubsystem.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Lab/TripoMenuStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/CoreStyle.h"
ATripoStoryTrigger::ATripoStoryTrigger()
{
    Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume")); SetRootComponent(Volume); Volume->SetBoxExtent(FVector(90,150,120));
    Volume->SetCollisionEnabled(ECollisionEnabled::QueryOnly); Volume->SetCollisionResponseToAllChannels(ECR_Ignore); Volume->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Hint")); Label->SetupAttachment(Volume); Label->SetRelativeLocation(FVector(0,0,160)); Label->SetWorldSize(26);
    WallNotice=CreateDefaultSubobject<UWidgetComponent>(TEXT("WallNotice"));
    WallNotice->SetupAttachment(Volume);
    WallNotice->SetWidgetSpace(EWidgetSpace::World);
    WallNotice->SetDrawSize(FVector2D(700,300));
    WallNotice->SetRelativeScale3D(FVector(.18f));
    WallNotice->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WallNotice->SetGenerateOverlapEvents(false);
    WallNotice->SetTwoSided(false);
}
void ATripoStoryTrigger::BeginPlay()
{
    Super::BeginPlay(); Volume->OnComponentBeginOverlap.AddDynamic(this, &ATripoStoryTrigger::Enter);
    WallNotice->SetVisibility(bWallNotice);
    if (bWallNotice && Catalog)
    {
        if (const auto* Event=Catalog->Find(EventId))
        {
            Label->SetVisibility(false);
            WallNotice->SetSlateWidget(SNew(SBorder)
                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(FLinearColor(.025f,.055f,.046f,.9f)).Padding(28)
                [SNew(SVerticalBox)
                +SVerticalBox::Slot().AutoHeight().Padding(0,0,0,16)[TripoMenu::Label(Event->Title.ToString(),28,FLinearColor(.82f,.66f,.36f),true)]
                +SVerticalBox::Slot().FillHeight(1)[TripoMenu::Label(Event->Text.ToString().Replace(TEXT("\\n"),TEXT("\n")),24,TripoMenu::Paper)]]);
        }
    }
}
void ATripoStoryTrigger::OnConstruction(const FTransform& T) { Super::OnConstruction(T); Label->SetText(FText::FromString(Hint)); }
bool ATripoStoryTrigger::Interact(ATripoCharacter* Player)
{
    if (!IsValid(Player) || FVector::DistSquared(Player->GetActorLocation(), GetActorLocation()) > FMath::Square(250.)) return false;
    FHitResult Hit; FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoStoryInteraction), false, Player);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Player->GetActorLocation(), GetActorLocation() + FVector(0,0,80), ECC_Visibility, Query) && Hit.GetActor() != this) return false;
    auto* Story=GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    const bool bOpened=Story->OpenEvent(Player, Catalog, EventId);
    if (bOpened && bWallNotice) Story->CloseEvent(false);
    return bOpened;
}
void ATripoStoryTrigger::Enter(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    auto* Player = Cast<ATripoCharacter>(Other);
    if (Player && bAutomatic && !UTripoProgressSubsystem::Get(Player)->HasApplied(EventId)) Interact(Player);
}
