#include "World/TripoSchoolBooks.h"
#include "World/TripoInteractionTarget.h"
#include "Player/TripoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "Sound/SoundBase.h"
#include "Lab/TripoMenuStyle.h"
#include "Widgets/Text/STextBlock.h"

ATripoSchoolBookItem::ATripoSchoolBookItem()
{
    Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh")); SetRootComponent(Mesh);
    Mesh->SetCollisionProfileName(TEXT("BlockAll")); Mesh->SetMobility(EComponentMobility::Movable);
    Interaction=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("Interaction"));
    Interaction->SetupAttachment(Mesh); Interaction->Reach=300; Interaction->HighlightMesh=Mesh;
    Interaction->SetBoxExtent(FVector(25,25,10));
}
void ATripoSchoolBookItem::BeginPlay()
{
    Super::BeginPlay();
    const TCHAR* Names[]={TEXT("拿起语文课本"),TEXT("拿起数学课本"),TEXT("拿起英语课本"),TEXT("拿起钥匙")};
    Interaction->Prompt=FText::FromString(Names[FMath::Clamp(Subject,0,3)]);
    Interaction->OnInteract.AddDynamic(this,&ATripoSchoolBookItem::Collect);
}
void ATripoSchoolBookItem::Collect(ATripoCharacter* Player)
{
    if(Puzzle) Puzzle->TryCollect(this,Player);
}
ATripoSchoolBooks::ATripoSchoolBooks()
{
    PrimaryActorTick.bCanEverTick=true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Instruction=CreateDefaultSubobject<UWidgetComponent>(TEXT("Instruction"));
    CheckChinese=CreateDefaultSubobject<UWidgetComponent>(TEXT("CheckChinese"));
    CheckMath=CreateDefaultSubobject<UWidgetComponent>(TEXT("CheckMath"));
    CheckEnglish=CreateDefaultSubobject<UWidgetComponent>(TEXT("CheckEnglish"));
    for(auto* W:{Instruction.Get(),CheckChinese.Get(),CheckMath.Get(),CheckEnglish.Get()})
    {
        W->SetupAttachment(RootComponent); W->SetWidgetSpace(EWidgetSpace::World);
        W->SetBlendMode(EWidgetBlendMode::Transparent); W->SetBackgroundColor(FLinearColor::Transparent);
        W->SetCollisionEnabled(ECollisionEnabled::NoCollision); W->SetGenerateOverlapEvents(false);
        W->SetDrawSize(FVector2D(120,100)); W->SetRelativeScale3D(FVector(.35));
    }
    Instruction->SetDrawSize(FVector2D(850,90));
}
void ATripoSchoolBooks::BeginPlay()
{
    Super::BeginPlay();
    Instruction->SetSlateWidget(SNew(STextBlock).Font(TripoMenu::Font(32))
        .ColorAndOpacity(FLinearColor(.85,.83,.69)).Justification(ETextJustify::Center)
        .Text(FText::FromString(TEXT("上课前，把课本收好。"))));
    for(auto* W:{CheckChinese.Get(),CheckMath.Get(),CheckEnglish.Get()})
    {
        W->SetSlateWidget(SNew(STextBlock).Font(TripoMenu::Font(64)).Justification(ETextJustify::Center)
            .ColorAndOpacity(FLinearColor(.87,.86,.73)).Text(FText::FromString(TEXT("✓"))));
        W->SetVisibility(false);
    }
    if(RewardKey) { RewardKey->SetActorHiddenInGame(true); RewardKey->SetActorEnableCollision(false); RewardKey->Interaction->bEnabled=false; KeyStart=RewardKey->GetActorLocation(); }
    if(Drawer) DrawerStart=Drawer->GetActorLocation();
    SetActorTickEnabled(false);
}
void ATripoSchoolBooks::Say(const FString& Text)
{
    Hint=Text; HintUntil=GetWorld()->GetTimeSeconds()+4;
}
FString ATripoSchoolBooks::GetHint() const { return GetWorld()->GetTimeSeconds()<HintUntil ? Hint : FString(); }
bool ATripoSchoolBooks::TryCollect(ATripoSchoolBookItem* Item,ATripoCharacter* Player)
{
    if(!HasAuthority() || !IsValid(Item) || Item->Puzzle!=this || Item->bCollected || !Item->Interaction->CanInteract(Player)) return false;
    if(Item==RewardKey)
    {
        if(CollectedCount!=3 || RevealTime<.65f || bKeyCollected || !IsValid(LockedDoor)) return false;
        auto* Key=FindFProperty<FBoolProperty>(LockedDoor->GetClass(),TEXT("成功拾取钥匙"));
        if(!Key) return false;
        Key->SetPropertyValue_InContainer(LockedDoor,true); bKeyCollected=true;
        Say(TEXT("这把钥匙，应该能打开旁边的门。"));
    }
    else
    {
        if(!Books.IsValidIndex(Item->Subject) || Books[Item->Subject]!=Item) return false;
        if(Item->Subject!=CollectedCount) { Say(TEXT("先上哪节课来着？")); return false; }
        ++CollectedCount;
        UWidgetComponent* Checks[]={CheckChinese,CheckMath,CheckEnglish}; Checks[Item->Subject]->SetVisibility(true);
        if(Notes.IsValidIndex(Item->Subject) && Notes[Item->Subject]) UGameplayStatics::PlaySound2D(this,Notes[Item->Subject]);
        if(CollectedCount==3)
        {
            RevealTime=0; SetActorTickEnabled(true);
            if(CompletionSound) UGameplayStatics::PlaySoundAtLocation(this,CompletionSound,Drawer?Drawer->GetActorLocation():GetActorLocation());
            Say(TEXT("讲台那边，好像有什么打开了。"));
            if(RewardKey) RewardKey->SetActorHiddenInGame(false);
        }
    }
    Item->bCollected=true; Item->Interaction->bEnabled=false; Item->Interaction->SetFocused(false);
    Item->SetActorHiddenInGame(true); Item->SetActorEnableCollision(false);
    return true;
}
void ATripoSchoolBooks::Tick(float Dt)
{
    Super::Tick(Dt); RevealTime+=Dt;
    const float A=FMath::SmoothStep(0.f,1.f,FMath::Clamp(RevealTime/.65f,0.f,1.f));
    if(Drawer) Drawer->SetActorLocation(DrawerStart+DrawerTravel*A);
    if(RewardKey && !bKeyCollected) RewardKey->SetActorLocation(KeyStart+DrawerTravel*A);
    if(RevealTime>=.65f)
    {
        if(RewardKey && !bKeyCollected) { RewardKey->SetActorEnableCollision(true); RewardKey->Interaction->bEnabled=true; }
        SetActorTickEnabled(false);
    }
}
