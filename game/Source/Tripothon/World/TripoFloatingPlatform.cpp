#include "World/TripoFloatingPlatform.h"
#include "World/TripoWaterHazard.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Player/TripoCharacter.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
ATripoFloatingPlatform::ATripoFloatingPlatform()
{
    PrimaryActorTick.bCanEverTick=true; PrimaryActorTick.TickGroup=TG_PrePhysics;
    Deck=CreateDefaultSubobject<UBoxComponent>(TEXT("Deck")); SetRootComponent(Deck);
    Deck->SetBoxExtent(FVector(70,55,6)); Deck->SetMobility(EComponentMobility::Movable);
    Deck->SetCollisionProfileName(TEXT("BlockAll")); Deck->SetCanEverAffectNavigation(false);
    Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual")); Visual->SetupAttachment(Deck);
    Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision); Visual->SetCanEverAffectNavigation(false);
    Tags.Add(TEXT("TripoFloating"));
}
void ATripoFloatingPlatform::BeginPlay() { Super::BeginPlay(); Rest=GetActorTransform(); }
void ATripoFloatingPlatform::ResetBuoyancy()
{
    Offset=Speed=FVector::ZeroVector; SetActorTransform(Rest,false,nullptr,ETeleportType::TeleportPhysics);
    PreviousLoadCount=0; LastRippleTime=-10; LastRippleLoad=FVector::ZeroVector;
}
void ATripoFloatingPlatform::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running) return;
    FVector Load=FVector::ZeroVector; int32 Count=0;
    for (TActorIterator<ATripoCharacter> It(GetWorld());It;++It)
        if (It->GetMovementBase()==Deck && It->GetCharacterMovement()->IsMovingOnGround())
        { Load+=Rest.InverseTransformPosition(It->GetActorLocation()); ++Count; }
    const float T=R->GetActionSeconds();
    const FVector AverageLoad=Count ? Load/Count : FVector::ZeroVector;
    const bool bLanding=Count>PreviousLoadCount;
    const bool bLeaving=Count<PreviousLoadCount;
    const bool bWalking=Count>0 && FVector::Dist2D(AverageLoad,LastRippleLoad)>12.f;
    // Landing gives a strong pulse; shifting weight and standing give quieter ripples.
    if (((bLanding || bLeaving) && T-LastRippleTime>.15) || (Count>0 && T-LastRippleTime>(bWalking ? .45 : 1.35)))
    {
        for (TActorIterator<ATripoWaterHazard> It(GetWorld());It;++It)
            It->AddRipple(GetActorLocation(),bLanding ? 1.f : (bLeaving ? .7f : (bWalking ? .4f : .12f)));
        LastRippleTime=T; LastRippleLoad=AverageLoad;
    }
    PreviousLoadCount=Count;
    const FVector Ext=Deck->GetUnscaledBoxExtent();
    const FVector Target=Count ? FVector(-SinkDepth,FMath::Clamp(Load.X/Count/Ext.X,-1.,1.)*MaxTilt,-FMath::Clamp(Load.Y/Count/Ext.Y,-1.,1.)*MaxTilt) : FVector::ZeroVector;
    // Substeps keep the damped spring stable during low frame rates.
    float Remaining=FMath::Min(DeltaSeconds,.1f);
    while (Remaining>0) { const float Dt=FMath::Min(Remaining,1.f/120); Speed+=(42.f*(Target-Offset)-9.f*Speed)*Dt; Offset+=Speed*Dt; Remaining-=Dt; }
    const float Phase=Rest.GetLocation().X*.013f;
    const FVector Location=Rest.GetLocation()+FVector(0,0,Offset.X+BobHeight*FMath::Sin(T*1.3f+Phase));
    const FRotator Rotation=Rest.Rotator()+FRotator(Offset.Y+.5f*FMath::Sin(T+Phase),0,Offset.Z+.5f*FMath::Cos(T*.8f+Phase));
    SetActorLocationAndRotation(Location,Rotation,false);
}

