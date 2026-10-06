#include "World/TripoChapterGift.h"
#include "World/TripoInteractionTarget.h"
#include "World/TripoWorldSubsystem.h"
#include "Lab/TripoHUD.h"
#include "Player/TripoCharacter.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/CharacterMovementComponent.h"
ATripoChapterGift::ATripoChapterGift()
{
    Portal=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChapterPortal"));
    Portal->SetupAttachment(RootComponent);
    Portal->SetRelativeLocation(FVector(0,0,115));
    Portal->SetRelativeRotation(FRotator(0,0,90));
    Portal->SetRelativeScale3D(FVector(2.6));
    Portal->SetCollisionEnabled(ECollisionEnabled::NoCollision); Portal->SetCastShadow(false);
    Portal->SetVisibility(false);
    PortalInteraction=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("PortalInteraction"));
    PortalInteraction->SetupAttachment(Portal);
    PortalInteraction->SetBoxExtent(FVector(48,48,8));
    PortalInteraction->Reach=300;
    PortalInteraction->Prompt=FText::FromString(TEXT("进入屏幕中的传送门"));
    PortalInteraction->bEnabled=false;
    PortalInteraction->HighlightMaterial=nullptr;
}
bool ATripoChapterGift::CanUsePortal() const { return bScreenPortal && bPortalReady && Reveal>=1 && !bTravelling; }
bool ATripoChapterGift::EnterPortal(ATripoCharacter* Player)
{
    if(!HasAuthority() || !CanUsePortal() || !PortalInteraction->CanInteract(Player)) return false;
    auto* P=UTripoProgressSubsystem::Get(this);
    if(!P || P->HasPendingLoad()) return false;
    bTravelling=P->Travel(Player,Destination);
    return bTravelling;
}
void ATripoChapterGift::BeginPlay()
{
    Super::BeginPlay(); PortalMaterial=Portal->CreateDynamicMaterialInstance(0);
    if (bOpened) Reveal=1;
    RefreshPortal(0);
}
TArray<int32> ATripoChapterGift::GetChoices() const
{
    auto* P=UTripoProgressSubsystem::Get(this);
    return P ? P->GetChapterGiftChoices(GetReceiptKey()) : TArray<int32>();
}
bool ATripoChapterGift::TryOpen(ATripoCharacter* Player)
{
    auto* P=UTripoProgressSubsystem::Get(this); auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if (!bEnabled || (bFadeToRoom && !IsValid(ArrivalPoint)) || !HasAuthority() || !IsInReach(Player) || !P || !R || R->IsActionPaused() ||
        R->GetRestorePhase()!=ETripoRestorePhase::Running || GiftId.IsNone() || P->HasClaimedGift(GetReceiptKey()) ||
        P->GetPhase()==ETripoChallengePhase::PendingReward || P->GetPhase()==ETripoChallengePhase::Running) return false;
    auto* PC=Cast<APlayerController>(Player->GetController()); auto* HUD=PC ? Cast<ATripoHUD>(PC->GetHUD()) : nullptr;
    if (!HUD || HUD->IsGameplayBlocked()) return false;
    ChoosingPlayer=Player; HUD->ShowChapterGift(this); return true;
}
bool ATripoChapterGift::ChooseSkill(ATripoCharacter* Player,int32 Ability)
{
    if ((bFadeToRoom && !IsValid(ArrivalPoint)) || !IsValid(Player) || ChoosingPlayer.Get()!=Player || !IsInReach(Player)) return false;
    auto* P=UTripoProgressSubsystem::Get(this);
    if (!P || !P->ClaimChapterGift(Player,GetReceiptKey(),Ability,Receipt)) return false;
    bOpened=true; ChoosingPlayer.Reset();
    TArray<int32> Floors; Floors.Init(0,8);
    P->ApplyStory(Player,CompletionEvent,Floors);
    P->MarkViewed(CompletionEvent);
    UTripoWorldSubsystem::Get(this)->SetCheckpoint(Player,Player->GetActorTransform());
    if (bFadeToRoom)
    {
        RoomFade=0; bRoomArrived=false;
        if (auto* PC=Cast<APlayerController>(Player->GetController()))
        { PC->SetIgnoreMoveInput(true); PC->SetIgnoreLookInput(true); PC->PlayerCameraManager->StartCameraFade(0,1,.5f,FLinearColor::Black,false,true); }
    }
    OnGiftOpened.Broadcast(Receipt);
    return true;
}
void ATripoChapterGift::RefreshPortal(float Dt)
{
    Prompt->SetVisibility(false);
    bPortalReady=bOpened;
    if (!bPortalReady) bEntryArmed=false;
    Reveal=FMath::FInterpConstantTo(Reveal,bPortalReady?1.f:0.f,Dt,1.5f);
    BoxMesh->SetVisibility(Reveal<.3f); LidMesh->SetVisibility(Reveal<.3f);
    BoxMesh->SetCollisionEnabled(bPortalReady?ECollisionEnabled::NoCollision:ECollisionEnabled::QueryAndPhysics);
    TInlineComponentArray<UTripoInteractionTarget*> Targets(this);
    for(auto* F:Targets) if(F!=PortalInteraction) { F->bEnabled=bEnabled && !bPortalReady; if(!F->bEnabled) F->SetFocused(false); }
    PortalInteraction->bEnabled=CanUsePortal();
    Portal->SetVisibility(bPortalReady && !bFadeToRoom); Portal->SetRelativeScale3D(FVector(PortalScale*FMath::Max(.01f,Reveal)));
}
void ATripoChapterGift::Tick(float Dt)
{
    Super::Tick(Dt);
    auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running) return;
    RefreshPortal(Dt);
    const double Now=R->GetActionSeconds();
    if (PortalMaterial) PortalMaterial->SetScalarParameterValue(TEXT("PortalTime"),Now);
    if(bFadeToRoom)
    {
        if(RoomFade>=0)
        {
            RoomFade+=Dt;
            auto* C=Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
            auto* PC=C?Cast<APlayerController>(C->GetController()):nullptr;
            if(PC && !bRoomArrived && RoomFade>=.55f)
            {
                if(IsValid(ArrivalPoint) && C->TeleportTo(ArrivalPoint->GetActorLocation(),ArrivalPoint->GetActorRotation()))
                { C->Abilities->CancelAll(); C->GetCharacterMovement()->StopMovementImmediately(); PC->SetControlRotation(ArrivalPoint->GetActorRotation()); UTripoWorldSubsystem::Get(this)->SetCheckpoint(C,C->GetActorTransform()); }
                bRoomArrived=true; PC->PlayerCameraManager->StartCameraFade(1,0,.65f,FLinearColor::Black,false,false);
            }
            if(PC && RoomFade>=1.25f) { PC->ResetIgnoreMoveInput(); PC->ResetIgnoreLookInput(); RoomFade=-1; }
        }
        return;
    }
    if(bScreenPortal) return;
    auto* Player=Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    auto* P=UTripoProgressSubsystem::Get(this);
    if (P && P->HasPendingLoad()) { bEntryArmed=false; return; }
    if (!bPortalReady || !Player || Player->bDebugFlying || bTravelling || Reveal<1) return;
    const FVector Local=Portal->GetComponentQuat().UnrotateVector(Player->GetActorLocation()-Portal->GetComponentLocation());
    // Test the actual portal plane, including a designer-authored wall placement.
    const bool bInside=FMath::Abs(Local.Z)<75 && FMath::Square(Local.X)+FMath::Square(Local.Y)<FMath::Square(100.f);
    if (!bInside) { bEntryArmed=true; return; }
    if (!bEntryArmed || Now<RetryAfter) return;
    const FName Map=Destination.IsNone()?FName(*P->SecondChapterMap):Destination;
    bTravelling=P->Travel(Player,Map); RetryAfter=Now+1;
}
