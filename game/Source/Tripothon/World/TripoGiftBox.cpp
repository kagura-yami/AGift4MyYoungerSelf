#include "World/TripoGiftBox.h"
#include "World/TripoInteractionTarget.h"
#include "Lab/TripoHUD.h"
#include "GameFramework/PlayerController.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ATripoGiftBox::ATripoGiftBox()
{
    PrimaryActorTick.bCanEverTick=true;
    PrimaryActorTick.bTickEvenWhenPaused=true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    BoxMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
    BoxMesh->SetupAttachment(RootComponent);
    BoxMesh->SetRelativeLocation(FVector(0,0,45));
    BoxMesh->SetRelativeScale3D(FVector(.8,.8,.9));
    BoxMesh->SetCollisionProfileName(TEXT("BlockAll"));
    auto* Focus=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("FocusTarget"));
    Focus->SetupAttachment(RootComponent); Focus->SetRelativeLocation(FVector(0,0,50)); Focus->SetBoxExtent(FVector(45,45,52));
    Focus->HighlightMesh=BoxMesh; Focus->Prompt=FText::FromString(TEXT("打开礼物"));
    LidMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMesh"));
    LidMesh->SetupAttachment(RootComponent);
    LidMesh->SetRelativeLocation(FVector(0,0,96));
    LidMesh->SetRelativeScale3D(FVector(.88,.88,.12));
    LidMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LidMesh->SetCanEverAffectNavigation(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) { BoxMesh->SetStaticMesh(Cube.Object); LidMesh->SetStaticMesh(Cube.Object); }
    Prompt=CreateDefaultSubobject<UTextRenderComponent>(TEXT("Prompt"));
    Prompt->SetupAttachment(RootComponent);
    Prompt->SetRelativeLocation(FVector(0,0,220));
    Prompt->SetHorizontalAlignment(EHTA_Center);
    Prompt->SetWorldSize(18);
    Prompt->SetText(FText::FromString(TEXT("E | RANDOM GIFT")));
    for (uint8 I=0; I<8; ++I) { FTripoRewardOption Option; Option.Ability=ETripoAbility(I); Rewards.Add(Option); }
}
void ATripoGiftBox::GenerateNewGiftId()
{
#if WITH_EDITOR
    if (GetWorld() && !GetWorld()->IsGameWorld()) { Modify(); GiftId=FName(*FGuid::NewGuid().ToString(EGuidFormats::Digits)); }
#endif
}
FName ATripoGiftBox::GetReceiptKey() const
{
    if (GiftId.IsNone() || !GetWorld()) return NAME_None;
    FString Package=GetWorld()->GetOutermost()->GetName();
    const FString Leaf=FPackageName::GetShortName(Package);
    if (Leaf.StartsWith(TEXT("UEDPIE_")))
    {
        const int32 End=Leaf.Find(TEXT("_"),ESearchCase::CaseSensitive,ESearchDir::FromStart,7);
        if (End!=INDEX_NONE) Package=FPackageName::GetLongPackagePath(Package)/Leaf.Mid(End+1);
    }
    return FName(*(Package+TEXT(":Gift:")+GiftId.ToString()));
}
void ATripoGiftBox::BeginPlay()
{
    Super::BeginPlay();
    ClosedLidLocation=LidMesh->GetRelativeLocation();
    ClosedLidRotation=LidMesh->GetRelativeRotation();
    RefreshReceipt(true);
    UpdateVisuals();
}
void ATripoGiftBox::RefreshReceipt(bool bInstant)
{
    if (auto* Progress=UTripoProgressSubsystem::Get(this))
    {
        bOpened=Progress->HasClaimedGift(GetReceiptKey());
        if (bOpened) Receipt=Progress->GetGiftReceipt(GetReceiptKey());
        if (bInstant) OpenAlpha=bOpened ? 1.f : 0.f;
    }
}
bool ATripoGiftBox::IsInReach(ATripoCharacter* Player) const
{
    if (!IsValid(Player) || !bEnabled || FVector::DistSquared(Player->GetActorLocation(),GetActorLocation())>FMath::Square(FMath::Max(50.f,InteractionDistance))) return false;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(GiftReach),false,this);
    Query.AddIgnoredActor(Player);
    // Imported gift props have a bottom pivot resting on furniture. Aim at the
    // visible mesh centre so the tabletop does not occlude the interaction.
    FVector Eye=Player->GetPawnViewLocation(); FRotator View;
    if (Player->GetController()) Player->GetController()->GetPlayerViewPoint(Eye,View);
    return !GetWorld()->LineTraceTestByChannel(Eye,BoxMesh->Bounds.Origin,ECC_Visibility,Query);
}
ATripoGiftBox* ATripoGiftBox::FindNearby(ATripoCharacter* Player)
{
    if (!IsValid(Player)) return nullptr;
    ATripoGiftBox* Best=nullptr;
    double Distance=TNumericLimits<double>::Max();
    for (TActorIterator<ATripoGiftBox> It(Player->GetWorld()); It; ++It)
    {
        if (It->bOpened || !It->IsInReach(Player)) continue;
        const double D=FVector::DistSquared(Player->GetActorLocation(),It->GetActorLocation());
        if (D<Distance) { Distance=D; Best=*It; }
    }
    return Best;
}
bool ATripoGiftBox::TryOpen(ATripoCharacter* Player)
{
    if (!HasAuthority() || !IsInReach(Player)) return false;
    RefreshReceipt(false);
    if (bOpened) { LastError=TEXT("礼物已经领取"); return false; }
    if (GiftId.IsNone()) { LastError=TEXT("请为礼物盒配置唯一 GiftId"); return false; }
    auto* Progress=UTripoProgressSubsystem::Get(this);
    if (!Progress || !Progress->ClaimGift(Player,GetReceiptKey(),Rewards,Receipt))
    { LastError=TEXT("当前不能领取：检查暂停、角色状态或奖池配置"); return false; }
    bOpened=true;
    LastError.Empty();
    OpenPresentationStart=FPlatformTime::Seconds();
    if (auto* PC=Cast<APlayerController>(Player->GetController()))
        if (auto* HUD=Cast<ATripoHUD>(PC->GetHUD())) HUD->ShowGiftReceipt(Receipt.AbilityIndex,Receipt.GrantedLevel,OpenSeconds,Rewards.Num()>1);
    OnGiftOpened.Broadcast(Receipt);
    UpdateVisuals();
    return true;
}
void ATripoGiftBox::Tick(float Dt)
{
    Super::Tick(Dt);
    // Also handles boxes sharing an ID and loads applied after actor BeginPlay.
    RefreshReceipt(false);
    const auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || (!R->IsActionPaused() && R->GetRestorePhase()==ETripoRestorePhase::Running))
        OpenAlpha=FMath::FInterpConstantTo(OpenAlpha,bOpened ? 1.f : 0.f,Dt,1.f/FMath::Max(.05f,OpenSeconds));
    // Cosmetic opening continues on real time while the reward pauses gameplay.
    if (bOpened && OpenPresentationStart>0 && R && R->HasPauseReason(ETripoPauseReason::Reward))
        OpenAlpha=FMath::Clamp(float((FPlatformTime::Seconds()-OpenPresentationStart)/FMath::Max(.05f,OpenSeconds)),0.f,1.f);
    UpdateVisuals();
}
void ATripoGiftBox::UpdateVisuals()
{
    const float Ease=OpenAlpha*OpenAlpha*(3.f-2.f*OpenAlpha);
    if (const UStaticMesh* Mesh=LidMesh->GetStaticMesh())
    {
        // Rotate around the rear rim, not the imported mesh's bottom pivot.
        // The lid remains attached to the box after the reward is collected.
        const FBox Bounds=Mesh->GetBoundingBox();
        const FVector Hinge=FVector(Bounds.Min.X,Bounds.GetCenter().Y,Bounds.Min.Z)*LidMesh->GetRelativeScale3D();
        const FQuat Closed=ClosedLidRotation.Quaternion();
        const FQuat Open=Closed*FQuat(FVector::YAxisVector,FMath::DegreesToRadians(-100.f*Ease));
        LidMesh->SetRelativeRotation(Open);
        LidMesh->SetRelativeLocation(ClosedLidLocation+Closed.RotateVector(Hinge)-Open.RotateVector(Hinge));
    }
    auto* Player=Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    Prompt->SetVisibility(IsValid(Player) && FVector::DistSquared(Player->GetActorLocation(),GetActorLocation())<FMath::Square(FMath::Max(50.f,InteractionDistance)+60.f));
    if (auto* Camera=UGameplayStatics::GetPlayerCameraManager(this,0))
        Prompt->SetWorldRotation((Camera->GetCameraLocation()-Prompt->GetComponentLocation()).Rotation());
    Prompt->SetTextRenderColor(bOpened ? FColor(130,210,160) : FColor(255,218,135));
    if (GiftId.IsNone()) Prompt->SetText(FText::FromString(TEXT("CONFIGURE GiftId")));
    else if (bOpened) Prompt->SetText(FText::FromString(Receipt.AbilityIndex==INDEX_NONE ? TEXT("OPENED | KEEPSAKE") : FString::Printf(TEXT("OPENED | Lv.%d"),Receipt.GrantedLevel)));
    else Prompt->SetText(FText::FromString(IsInReach(Player) ? TEXT("[ E ] OPEN GIFT") : TEXT("RANDOM GIFT")));
}
