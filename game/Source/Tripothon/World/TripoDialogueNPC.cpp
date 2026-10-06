#include "World/TripoDialogueNPC.h"
#include "World/TripoInteractionTarget.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Story/TripoStorySubsystem.h"
#include "TimerManager.h"

ATripoDialogueNPC::ATripoDialogueNPC()
{
    PrimaryActorTick.bCanEverTick=false;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Body=CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
    Body->SetupAttachment(RootComponent); Body->SetRelativeLocation(FVector(0,0,88));
    Body->InitCapsuleSize(34,88); Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Body->SetCollisionResponseToChannel(ECC_Visibility,ECR_Ignore);
    Body->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
    Visual=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Visual"));
    Visual->SetupAttachment(RootComponent); Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visual->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    Interaction=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("Interaction"));
    Interaction->SetupAttachment(RootComponent); Interaction->SetRelativeLocation(FVector(0,0,90));
    Interaction->SetBoxExtent(FVector(42,42,90)); Interaction->Reach=280;
    Interaction->HighlightMesh=Visual; Interaction->bNPCConversation=true;
    Interaction->Prompt=FText::FromString(TEXT("与西装男交谈"));
}
void ATripoDialogueNPC::BeginPlay()
{
    Super::BeginPlay();
    Interaction->OnInteract.AddDynamic(this,&ATripoDialogueNPC::Talk);
    RestoreIdle();
}
void ATripoDialogueNPC::EndPlay(const EEndPlayReason::Type Reason)
{
    GetWorldTimerManager().ClearTimer(AnimationTimer);
    Super::EndPlay(Reason);
}
void ATripoDialogueNPC::PlaySequence(UAnimSequence* Animation,bool bLoop)
{
    if(!Animation) return;
    if(auto* Instance=Visual->GetSingleNodeInstance())
    {
        Instance->SetAnimationAsset(Animation,bLoop,1.f);
        Instance->SetPosition(0,false); Instance->SetPlaying(true);
    }
    else Visual->PlayAnimation(Animation,bLoop);
}
void ATripoDialogueNPC::RestoreIdle() { PlaySequence(IdleAnimation,true); }
void ATripoDialogueNPC::Talk(ATripoCharacter* Player)
{
    if(!HasAuthority() || !Interaction->CanInteract(Player)) return;
    auto* Story=GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    if(!Story || !Story->OpenEvent(Player,DialogueCatalog,DialogueEvent)) return;
    Story->SetWorldSpeaker(this);
    ++ConversationCount;
    GetWorldTimerManager().ClearTimer(AnimationTimer);
    if(TalkAnimation)
    {
        PlaySequence(TalkAnimation,false);
        GetWorldTimerManager().SetTimer(AnimationTimer,this,&ATripoDialogueNPC::RestoreIdle,FMath::Max(.01f,TalkAnimation->GetPlayLength()),false);
    }
    OnConversationStarted(Player);
}
