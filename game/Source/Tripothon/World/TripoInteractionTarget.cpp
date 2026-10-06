#include "World/TripoInteractionTarget.h"
#include "Story/TripoStorySubsystem.h"
#include "Components/WidgetComponent.h"
#include "Lab/TripoMenuStyle.h"
#include "Kismet/GameplayStatics.h"
#include "World/TripoNPCLookAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "World/TripoElevator.h"
#include "World/TripoGiftBox.h"
#include "World/TripoChapterGift.h"
#include "World/TripoInteractorComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/TripoMechanism.h"
#include "Components/TimelineComponent.h"
#include "UObject/UnrealType.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
UTripoInteractionTarget::UTripoInteractionTarget()
{
    PrimaryComponentTick.bCanEverTick=true;
    PrimaryComponentTick.bStartWithTickEnabled=false;
    PrimaryComponentTick.TickInterval=.1f;
    InitBoxExtent(FVector(10,12,12));
    BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BodyInstance.SetResponseToAllChannels(ECR_Ignore);
    BodyInstance.SetResponseToChannel(ECC_Visibility,ECR_Block);
    SetGenerateOverlapEvents(false); SetCanEverAffectNavigation(false);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/Materials/Whitebox/M_InteractionFocus.M_InteractionFocus"));
    HighlightMaterial=Glow.Object;
    Prompt=FText::FromString(TEXT("交互"));
}
void UTripoInteractionTarget::BeginPlay()
{
    Super::BeginPlay();
    SetComponentTickEnabled(bUseSchoolKey || bNPCConversation);
    if(bNPCConversation)
    {
        PrimaryComponentTick.TickInterval=0.f;
        if(auto* Mesh=Cast<USkeletalMeshComponent>(HighlightMesh))
        {
            // Only replace single-sequence playback, never an authored animation Blueprint.
            if(auto* Idle=Mesh->GetSingleNodeInstance())
            {
                auto* Asset=Idle->GetCurrentAsset();
                const float Position=Idle->GetCurrentTime();
                Mesh->SetAnimInstanceClass(UTripoNPCLookAnimInstance::StaticClass());
                LookAnimation=Cast<UTripoNPCLookAnimInstance>(Mesh->GetAnimInstance());
                if(LookAnimation) { LookAnimation->SetAnimationAsset(Asset); LookAnimation->SetPosition(Position,false); }
            }
        }
        SpeechBubble=NewObject<UWidgetComponent>(GetOwner());
        SpeechBubble->SetupAttachment(GetOwner()->GetRootComponent());
        SpeechBubble->SetWidgetSpace(EWidgetSpace::Screen);
        SpeechBubble->SetDrawSize(FVector2D(340,140));
        SpeechBubble->SetPivot(FVector2D(.5f,1.f));
        SpeechBubble->SetRelativeScale3D(FVector(.22f));
        SpeechBubble->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SpeechBubble->SetGenerateOverlapEvents(false);
        SpeechBubble->SetTwoSided(true);
        SpeechBubble->RegisterComponent();
        static const FSlateRoundedBoxBrush Paper(FLinearColor(.98f,.98f,.96f),18.f);
        const TWeakObjectPtr<UTripoInteractionTarget> WeakThis(this);
        auto Text=SNew(STextBlock).Font(TripoMenu::Font(17)).ColorAndOpacity(FLinearColor(.045f,.05f,.055f))
            .WrapTextAt(300).LineHeightPercentage(1.2f).Text_Lambda([WeakThis]()
            {
                if(!WeakThis.IsValid()) return FText::GetEmpty();
                FString Line=WeakThis->GetWorld()->GetSubsystem<UTripoStorySubsystem>()->GetLine();
                int32 Split;
                if(Line.FindChar(TEXT('：'),Split) && Split<16) Line=Line.Mid(Split+1);
                return FText::FromString(Line);
            });
        SpeechBubble->SetSlateWidget(SNew(SVerticalBox)
            +SVerticalBox::Slot().FillHeight(1)
            +SVerticalBox::Slot().AutoHeight()[SNew(SBorder).BorderImage(&Paper).Padding(FMargin(20,14))[Text]]
            +SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[TripoMenu::Label(TEXT("▼"),14,FLinearColor(.98f,.98f,.96f),false,false)]);
        SpeechBubble->GetSlateWidget()->SetVisibility(EVisibility::HitTestInvisible);
        SpeechBubble->SetVisibility(false);
    }
}
void UTripoInteractionTarget::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn)
{
    Super::TickComponent(Dt,Type,Fn);
    if(SpeechBubble)
    {
        auto* Story=GetWorld()->GetSubsystem<UTripoStorySubsystem>();
        const bool bSpeaking=Story->GetWorldSpeaker()==GetOwner();
        BubbleOpacity=FMath::FInterpConstantTo(BubbleOpacity,bSpeaking?1.f:0.f,Dt,6.f);
        SpeechBubble->SetVisibility(BubbleOpacity>0.f);
        SpeechBubble->GetSlateWidget()->SetRenderOpacity(BubbleOpacity);
        SpeechBubble->SetWorldLocation(GetOwner()->GetActorLocation()+FVector(0,0,200));
        if(auto* PC=UGameplayStatics::GetPlayerController(this,0))
        {
            FVector Camera; FRotator View; PC->GetPlayerViewPoint(Camera,View);
            FHitResult Hit;
            FCollisionQueryParams Q(SCENE_QUERY_STAT(NPCSpeechVisibility),false,GetOwner());
            Q.AddIgnoredActor(PC->GetPawn());
            const bool bOccluded=GetWorld()->LineTraceSingleByChannel(Hit,Camera,SpeechBubble->GetComponentLocation(),ECC_Visibility,Q);
            SpeechBubble->SetVisibility(BubbleOpacity>0.f && !bOccluded);
        }
        if(auto* Player=Cast<ATripoCharacter>(UGameplayStatics::GetPlayerCharacter(this,0)); Player && LookAnimation)
        {
            const FVector Direction=Player->GetActorLocation()-GetOwner()->GetActorLocation();
            FCollisionQueryParams GazeQuery(SCENE_QUERY_STAT(NPCGaze),false,GetOwner());
            GazeQuery.AddIgnoredActor(Player);
            FHitResult Obstruction;
            const FVector Eyes=HighlightMesh->GetSocketLocation(TEXT("head"));
            const bool bWatching=Direction.SizeSquared()<FMath::Square(650.f) &&
                !GetWorld()->LineTraceSingleByChannel(Obstruction,Eyes,Player->GetActorLocation()+FVector(0,0,35),ECC_Visibility,GazeQuery);
            float Yaw=0,Pitch=0;
            if(bWatching)
            {
                const float DesiredBody=Direction.Rotation().Yaw-90.f;
                const float Error=FMath::FindDeltaAngleDegrees(GetOwner()->GetActorRotation().Yaw,DesiredBody);
                if(FMath::Abs(Error)>45.f) BodyTurnDelay+=Dt; else BodyTurnDelay=0.f;
                if(BodyTurnDelay>.35f) bBodyTurning=true;
                if(FMath::Abs(Error)<12.f) bBodyTurning=false;
                const float DesiredSpeed=bBodyTurning?FMath::Clamp(Error*1.4f,-50.f,50.f):0.f;
                BodyTurnSpeed=FMath::FInterpTo(BodyTurnSpeed,DesiredSpeed,Dt,3.f);
                GetOwner()->AddActorWorldRotation(FRotator(0,BodyTurnSpeed*Dt,0));
                Yaw=FMath::Clamp(FMath::FindDeltaAngleDegrees(GetOwner()->GetActorRotation().Yaw,DesiredBody),-65.f,65.f);
                Pitch=FMath::Clamp((Player->GetActorLocation()+FVector(0,0,35)-Eyes).Rotation().Pitch,-28.f,18.f);
            }
            else { BodyTurnDelay=0; bBodyTurning=false; BodyTurnSpeed=0; }
            LookAnimation->GazeYaw=FMath::FInterpTo(LookAnimation->GazeYaw,Yaw,Dt,6.f);
            LookAnimation->GazePitch=FMath::FInterpTo(LookAnimation->GazePitch,Pitch,Dt,5.f);
            LookAnimation->TorsoYaw=FMath::FInterpTo(LookAnimation->TorsoYaw,Yaw*.22f,Dt,2.5f);
        }
    }
    if(!bUseSchoolKey || bSchoolDoorUnlocked) return;
    const auto* Key=FindFProperty<FBoolProperty>(GetOwner()->GetClass(),TEXT("成功拾取钥匙"));
    Prompt=FText::FromString(Key && Key->GetPropertyValue_InContainer(GetOwner()) ? TEXT("使用钥匙开门") : TEXT("需要对应钥匙"));
}
bool UTripoInteractionTarget::CanInteract(ATripoCharacter* P) const
{
    const auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if(!bEnabled || !IsValid(P) || !P->IsPlayerControlled() || P->Interactor->bSuppressed || !R || R->IsActionPaused() ||
        R->GetRestorePhase()!=ETripoRestorePhase::Running || FVector::DistSquared(P->GetActorLocation(),GetComponentLocation())>FMath::Square(Reach)) return false;
    if(const auto* Lift=Cast<ATripoElevator>(GetOwner()); Lift && !Lift->CanUseTarget(this,P)) return false;
    const auto* Chapter=Cast<ATripoChapterGift>(GetOwner());
    if(Chapter && Chapter->PortalInteraction==this) { if(!Chapter->CanUsePortal()) return false; }
    else if(const auto* Gift=Cast<ATripoGiftBox>(GetOwner())) return Gift->bEnabled && !Gift->bOpened && Gift->IsInReach(P);
    if(const auto* Mechanism=Cast<ATripoMechanism>(GetOwner()); Mechanism && Mechanism->Kind!=ETripoMechanismKind::Switch) return false;
    FHitResult Hit; FCollisionQueryParams Q(SCENE_QUERY_STAT(FocusReach),false,P);
    FVector Start=P->GetActorLocation();
    if(Chapter && Chapter->PortalInteraction==this)
        if(auto* PC=Cast<APlayerController>(P->GetController())) { FRotator View; PC->GetPlayerViewPoint(Start,View); }
    return !GetWorld()->LineTraceSingleByChannel(Hit,Start,GetComponentLocation(),ECC_Visibility,Q) || Hit.GetComponent()==this || Hit.GetComponent()==HighlightMesh;
}
bool UTripoInteractionTarget::TryInteract(ATripoCharacter* P)
{
    if(!GetOwner()->HasAuthority() || !CanInteract(P)) return false;
    if(StoryCatalog && !StoryEvent.IsNone())
    {
        auto* Story=GetWorld()->GetSubsystem<UTripoStorySubsystem>();
        if(!Story->OpenEvent(P,StoryCatalog,StoryEvent)) return false;
        if(bNPCConversation) { Story->SetWorldSpeaker(GetOwner()); ConversationPlayer=P; }
        return true;
    }
    if(auto* Lift=Cast<ATripoElevator>(GetOwner())) return Lift->UseTarget(this,P);
    if(auto* Chapter=Cast<ATripoChapterGift>(GetOwner()); Chapter && Chapter->PortalInteraction==this) return Chapter->EnterPortal(P);
    if(auto* Gift=Cast<ATripoGiftBox>(GetOwner())) return Gift->TryOpen(P);
    if(auto* Mechanism=Cast<ATripoMechanism>(GetOwner())) return Mechanism->Interact(P);
    if (bToggleDoorTimeline)
    {
        auto* Timeline=GetOwner()->FindComponentByClass<UTimelineComponent>();
        if (!Timeline) return false;
        if(bUseSchoolKey && !bSchoolDoorUnlocked)
        {
            auto* Key=FindFProperty<FBoolProperty>(GetOwner()->GetClass(),TEXT("成功拾取钥匙"));
            auto* Target=FindFProperty<FNameProperty>(GetOwner()->GetClass(),TEXT("门目标"));
            if(!Key || !Target || !Key->GetPropertyValue_InContainer(GetOwner())) return false;
            const FName KeyId=Target->GetPropertyValue_InContainer(GetOwner());
            if(KeyId.IsNone()) return false;
            // The Blueprint pickup mirrors one key onto matching doors. Consume
            // every copy of that flag atomically; only this door stays unlocked.
            for(TActorIterator<AActor> It(GetWorld());It;++It)
            {
                auto* OtherKey=FindFProperty<FBoolProperty>(It->GetClass(),TEXT("成功拾取钥匙"));
                auto* OtherTarget=FindFProperty<FNameProperty>(It->GetClass(),TEXT("门目标"));
                if(OtherKey && OtherTarget && OtherTarget->GetPropertyValue_InContainer(*It)==KeyId)
                    OtherKey->SetPropertyValue_InContainer(*It,false);
            }
            bSchoolDoorUnlocked=true;
        }
        const bool bClose=Timeline->IsPlaying() ? !Timeline->IsReversing() : Timeline->GetPlaybackPosition()>0.f;
        if (bClose) Timeline->Reverse(); else Timeline->Play();
        Prompt=FText::FromString(bClose ? TEXT("开门") : TEXT("关门"));
        OnInteract.Broadcast(P);
        return true;
    }
    if (!InteractionEvent.IsNone())
    {
        UFunction* Event = GetOwner()->FindFunction(InteractionEvent);
        if (!Event || Event->NumParms != 0) return false;
        GetOwner()->ProcessEvent(Event,nullptr);
    }
    OnInteract.Broadcast(P);
    if (bSingleUse) { bEnabled=false; SetFocused(false); }
    return true;
}
void UTripoInteractionTarget::SetFocused(bool bFocused)
{
    if(bFocused==bHighlighted) return;
    bHighlighted=bFocused;
    if(!IsValid(HighlightMesh) || !HighlightMaterial) return;
    if(bFocused) { PreviousOverlay=HighlightMesh->GetOverlayMaterial(); HighlightMesh->SetOverlayMaterial(HighlightMaterial); }
    else { HighlightMesh->SetOverlayMaterial(PreviousOverlay); PreviousOverlay=nullptr; }
    if (auto* Gift=Cast<ATripoGiftBox>(GetOwner()); Gift && Gift->LidMesh)
    {
        if (bFocused) { PreviousLidOverlay=Gift->LidMesh->GetOverlayMaterial(); Gift->LidMesh->SetOverlayMaterial(HighlightMaterial); }
        else { Gift->LidMesh->SetOverlayMaterial(PreviousLidOverlay); PreviousLidOverlay=nullptr; }
    }
}
