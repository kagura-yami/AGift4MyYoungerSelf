#include "World/TripoOfficeCipher.h"
#include "World/TripoInteractionTarget.h"
#include "World/TripoChapterGift.h"
#include "World/TripoWorldSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Lab/TripoHUD.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"

ATripoOfficeCipher::ATripoOfficeCipher()
{
    PrimaryActorTick.bCanEverTick=true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Card=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CipherCard"));
    Book=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CipherBook"));
    Drawer=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CipherDrawer"));
    CardTarget=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("CardTarget"));
    BookTarget=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("BookTarget"));
    DrawerTarget=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("DrawerTarget"));
    for (auto* M:{Card.Get(),Book.Get(),Drawer.Get()})
    { M->SetupAttachment(RootComponent); M->SetMobility(EComponentMobility::Movable); M->SetCollisionProfileName(TEXT("BlockAllDynamic")); M->SetCanEverAffectNavigation(false); }
    CardTarget->SetupAttachment(Card); BookTarget->SetupAttachment(Book); DrawerTarget->SetupAttachment(Drawer);
    CardTarget->HighlightMesh=Card; BookTarget->HighlightMesh=Book; DrawerTarget->HighlightMesh=Drawer;
    for(auto* T:{CardTarget.Get(),BookTarget.Get(),DrawerTarget.Get()}) { T->Reach=280; T->SetBoxExtent(FVector(20)); }
    CardTarget->Prompt=FText::FromString(TEXT("取下打孔卡"));
    BookTarget->Prompt=FText::FromString(TEXT("翻开笔记"));
    DrawerTarget->Prompt=FText::FromString(TEXT("输入抽屉密码"));
    PageDigits={8,5,2,9, 4,8,6,0, 6,2,9,4, 0,6,3,8, 9,1,4,6, 2,7,5,3, 5,2,8,1};
}
void ATripoOfficeCipher::BeginPlay()
{
    Super::BeginPlay();
    DrawerClosed=Drawer->GetRelativeLocation(); if(Reward) RewardClosed=Reward->GetActorLocation();
    CardTarget->OnInteract.AddDynamic(this,&ATripoOfficeCipher::CardUsed);
    BookTarget->OnInteract.AddDynamic(this,&ATripoOfficeCipher::BookUsed);
    DrawerTarget->OnInteract.AddDynamic(this,&ATripoOfficeCipher::DrawerUsed);
    auto* P=UTripoProgressSubsystem::Get(this);
    bHasCard=P->HasApplied(FName(*(PuzzleId.ToString()+TEXT(".Card"))));
    bUnlocked=P->HasApplied(FName(*(PuzzleId.ToString()+TEXT(".Unlocked"))));
    OpenAlpha=bUnlocked?1.f:0.f; RefreshObjects();
}
bool ATripoOfficeCipher::InReach(ATripoCharacter* P,UTripoInteractionTarget* T) const
{ return HasAuthority() && IsValid(P) && P->IsPlayerControlled() && FVector::DistSquared(P->GetActorLocation(),T->GetComponentLocation())<=FMath::Square(T->Reach); }
void ATripoOfficeCipher::CardUsed(ATripoCharacter* P) { TakeCard(P); }
void ATripoOfficeCipher::BookUsed(ATripoCharacter* P) { OpenBook(P); }
void ATripoOfficeCipher::DrawerUsed(ATripoCharacter* P) { OpenLock(P); }
bool ATripoOfficeCipher::TakeCard(ATripoCharacter* P)
{
    if(bHasCard || !InReach(P,CardTarget)) return false;
    TArray<int32> Floors; Floors.Init(0,8);
    UTripoProgressSubsystem::Get(this)->ApplyStory(P,FName(*(PuzzleId.ToString()+TEXT(".Card"))),Floors);
    bHasCard=true; RefreshObjects(); return true;
}
bool ATripoOfficeCipher::OpenBook(ATripoCharacter* P)
{
    if(!InReach(P,BookTarget)) return false;
    auto* PC=Cast<APlayerController>(P->GetController()); auto* HUD=PC?Cast<ATripoHUD>(PC->GetHUD()):nullptr;
    if(!HUD || HUD->IsGameplayBlocked()) return false;
    Feedback.Empty(); HUD->ShowOfficeCipher(this,false); return true;
}
bool ATripoOfficeCipher::OpenLock(ATripoCharacter* P)
{
    if(bUnlocked || !InReach(P,DrawerTarget)) return false;
    auto* PC=Cast<APlayerController>(P->GetController()); auto* HUD=PC?Cast<ATripoHUD>(PC->GetHUD()):nullptr;
    if(!HUD || HUD->IsGameplayBlocked()) return false;
    Feedback.Empty(); HUD->ShowOfficeCipher(this,true); return true;
}
void ATripoOfficeCipher::SelectPage(int32 N) { Page=FMath::Clamp(N,1,7); }
void ATripoOfficeCipher::ToggleCard() { if(bHasCard) bCardOnPage=!bCardOnPage; }
int32 ATripoOfficeCipher::Digit(int32 N,int32 Hole) const
{ const int32 I=(N-1)*4+Hole; return N>=1 && N<=7 && Hole>=0 && Hole<4 && PageDigits.IsValidIndex(I)?FMath::Clamp(PageDigits[I],0,9):0; }
bool ATripoOfficeCipher::SubmitCode(ATripoCharacter* P,const FString& Code)
{
    if(bUnlocked || !bHasCard || !InReach(P,DrawerTarget)) { Feedback=TEXT("先找到留在白板上的线索。"); return false; }
    const FString Expected=FString::Printf(TEXT("%d%d%d%d"),Digit(3,0),Digit(7,1),Digit(5,2),Digit(1,3));
    if(Code!=Expected) { Feedback=TEXT("锁没有打开，再核对一下。"); return false; }
    TArray<int32> Floors; Floors.Init(0,8);
    UTripoProgressSubsystem::Get(this)->ApplyStory(P,FName(*(PuzzleId.ToString()+TEXT(".Unlocked"))),Floors);
    bUnlocked=true; Feedback=TEXT("咔哒。"); RefreshObjects();
    UTripoWorldSubsystem::Get(this)->SetCheckpoint(P,P->GetActorTransform()); return true;
}
void ATripoOfficeCipher::RefreshObjects()
{
    Card->SetVisibility(!bHasCard); Card->SetCollisionEnabled(bHasCard?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryAndPhysics);
    CardTarget->bEnabled=!bHasCard; DrawerTarget->bEnabled=!bUnlocked;
    CardTarget->SetCollisionEnabled(bHasCard?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryOnly);
    DrawerTarget->SetCollisionEnabled(bUnlocked?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryOnly);
    if(Reward) { Reward->bEnabled=bUnlocked && OpenAlpha>.95f; Reward->SetActorHiddenInGame(!Reward->bEnabled); Reward->SetActorEnableCollision(Reward->bEnabled); }
}
void ATripoOfficeCipher::Tick(float Dt)
{
    Super::Tick(Dt); OpenAlpha=FMath::FInterpConstantTo(OpenAlpha,bUnlocked?1.f:0.f,Dt,1.3f);
    const float T=OpenAlpha*OpenAlpha*(3-2*OpenAlpha);
    Drawer->SetRelativeLocation(DrawerClosed+DrawerTravel*T);
    if(Reward) Reward->SetActorLocation(RewardClosed+GetActorTransform().TransformVectorNoScale(DrawerTravel)*T);
    RefreshObjects();
}
