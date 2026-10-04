#include "World/TripoElevator.h"
#include "World/TripoInteractionTarget.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Player/TripoCharacter.h"
#include "World/TripoInteractorComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ATripoElevator::ATripoElevator()
{
    LandingButtonActors.SetNum(2); LandingButtonOffsets={FVector(0,13,24),FVector(0,13,8)};
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Identity = CreateDefaultSubobject<UTripoIdentityComponent>(TEXT("Identity"));
    Cabin = CreateDefaultSubobject<USceneComponent>(TEXT("Cabin")); Cabin->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto Mesh = [&](FName Name,USceneComponent* Parent,FVector Location,FVector Size)
    {
        auto* C = CreateDefaultSubobject<UStaticMeshComponent>(Name); C->SetupAttachment(Parent);
        C->SetMobility(EComponentMobility::Movable); C->SetRelativeLocation(Location); C->SetRelativeScale3D(Size/100);
        C->SetStaticMesh(Cube.Object); C->SetCollisionProfileName(TEXT("BlockAllDynamic")); C->SetCanEverAffectNavigation(false); return C;
    };
    Floor = Mesh(TEXT("Floor"),Cabin,FVector(0,0,-10),FVector(320,320,20));
    Mesh(TEXT("BackWall"),Cabin,FVector(155,0,140),FVector(10,320,280));
    Mesh(TEXT("SideWallLeft"),Cabin,FVector(0,-155,140),FVector(320,10,280));
    Mesh(TEXT("SideWallRight"),Cabin,FVector(0,155,140),FVector(320,10,280));
    LeftDoor = Mesh(TEXT("LeftDoor"),Cabin,FVector(-155,-50,130),FVector(10,100,260));
    RightDoor = Mesh(TEXT("RightDoor"),Cabin,FVector(-155,50,130),FVector(10,100,260));
    DoorSensor = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorSensor")); DoorSensor->SetupAttachment(Cabin);
    DoorSensor->SetRelativeLocation(FVector(-160,0,130)); DoorSensor->SetBoxExtent(FVector(95,125,140));
    CabinVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CabinVolume")); CabinVolume->SetupAttachment(Cabin);
    CabinVolume->SetRelativeLocation(FVector(0,0,130)); CabinVolume->SetBoxExtent(FVector(140,140,130));
    for (auto* C : {DoorSensor.Get(),CabinVolume.Get()}) { C->SetCollisionEnabled(ECollisionEnabled::NoCollision); C->SetCanEverAffectNavigation(false); }
    Button = Mesh(TEXT("Button"),Cabin,FVector(100,115,110),FVector(12,12,20));
    CabinControl=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("CabinControl"));
    CabinControl->SetupAttachment(Button); CabinControl->SetRelativeScale3D(FVector(100.f/12,100.f/12,5));
    CabinControl->SetBoxExtent(FVector(9,9,13)); CabinControl->HighlightMesh=Cast<UMeshComponent>(Button);
    CabinControl->Prompt=FText::FromString(TEXT("前往另一层"));
    DoorButton=Mesh(TEXT("DoorButton"),Cabin,FVector(100,115,145),FVector(12,12,20));
    DoorControl=CreateDefaultSubobject<UTripoInteractionTarget>(TEXT("DoorControl"));
    DoorControl->SetupAttachment(DoorButton);
    DoorControl->SetRelativeScale3D(FVector(100.f/12,100.f/12,5));
    DoorControl->SetBoxExtent(FVector(9,9,13));
    DoorControl->HighlightMesh=DoorButton;
    DoorControl->Prompt=FText::FromString(TEXT("开门 / 延长开门"));
    for (int32 I=0;I<2;++I)
    {
        auto* CallMesh=Mesh(*FString::Printf(TEXT("CallButton%d"),I),RootComponent,FVector(-175,140,110+600*I),FVector(8,16,16));
        auto* Control=CreateDefaultSubobject<UTripoInteractionTarget>(*FString::Printf(TEXT("LandingControl%d"),I));
        Control->SetupAttachment(CallMesh); Control->SetRelativeScale3D(FVector(12.5,6.25,6.25));
        Control->SetBoxExtent(FVector(6,10,10)); Control->HighlightMesh=CallMesh;
        Control->Prompt=FText::FromString(I ? TEXT("下行 / 呼叫电梯") : TEXT("上行 / 呼叫电梯")); LandingControls.Add(Control);
        for(int32 Side=0;Side<2;++Side) LandingDoors.Add(Mesh(*FString::Printf(TEXT("Landing%dDoor%d"),I,Side),RootComponent,FVector::ZeroVector,FVector(10,100,260)));
        auto* B = CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("Landing%dBarrier"),I));
        B->SetupAttachment(RootComponent); B->SetBoxExtent(FVector(8,105,135)); B->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        B->SetCanEverAffectNavigation(false); LandingBarriers.Add(B);
    }
}
void ATripoElevator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (!HasActorBegunPlay()) { CurrentFloor=FMath::Clamp(InitialFloor,0,1); Cabin->SetRelativeLocation(CurrentFloor ? Stop1 : Stop0); }
    UpdateControlAnchors();
    ApplyDoors();
}
void ATripoElevator::BeginPlay()
{
    Super::BeginPlay();
    if (!bRestoredBeforeBegin) RestoreFloor(InitialFloor);
    for (TActorIterator<ATripoCharacter> It(GetWorld());It;++It) It->AddTickPrerequisiteActor(this);
}
bool ATripoElevator::Contains(const UBoxComponent* Box,const ATripoCharacter* Player,bool bCapsule) const
{
    const FVector P=Box->GetComponentTransform().InverseTransformPosition(Player->GetActorLocation());
    FVector E=Box->GetUnscaledBoxExtent();
    if(bCapsule) { const auto* C=Player->GetCapsuleComponent(); const FVector S=Box->GetComponentScale().GetAbs(); E+=FVector(C->GetScaledCapsuleRadius(),C->GetScaledCapsuleRadius(),C->GetScaledCapsuleHalfHeight())/S; }
    return FMath::Abs(P.X)<=E.X && FMath::Abs(P.Y)<=E.Y && FMath::Abs(P.Z)<=E.Z;
}
bool ATripoElevator::DoorOccupied() const
{
    for(TActorIterator<ATripoCharacter> It(GetWorld());It;++It) if(Contains(DoorSensor,*It,true)) return true;
    return false;
}
bool ATripoElevator::CanInteract(ATripoCharacter* Player) const
{
    return CabinControl && CabinControl->CanInteract(Player);
}
bool ATripoElevator::Interact(ATripoCharacter* Player)
{
    return UseTarget(CabinControl,Player);
}
bool ATripoElevator::CanUseTarget(const UTripoInteractionTarget* Target, ATripoCharacter* Player) const
{
    if(Target==DoorControl)
        return Phase!=ETripoElevatorPhase::Moving && Contains(CabinVolume,Player,false) &&
            FVector::DistSquared(Player->GetActorLocation(),DoorButton->GetComponentLocation())<=FMath::Square(InteractionDistance);
    if(!IsStable()) return false;
    if(Target==CabinControl) return Contains(CabinVolume,Player,false) && FVector::DistSquared(Player->GetActorLocation(),Button->GetComponentLocation())<=FMath::Square(InteractionDistance);
    return LandingControls.Contains(Target);
}
bool ATripoElevator::UseTarget(UTripoInteractionTarget* Target, ATripoCharacter* Player)
{
    if(!Target || !Target->CanInteract(Player)) return false;
    if(Target==DoorControl)
    {
        // A separate open command can cancel departure while still docked.
        Phase=ETripoElevatorPhase::Docked; Destination=CurrentFloor;
        bDoorRequested=true; CloseRemaining=FMath::Max(5.f,AutoCloseDelay);
        return true;
    }
    if(Target==CabinControl) { Destination=1-CurrentFloor; Phase=ETripoElevatorPhase::ClosingForTravel; bDoorRequested=false; return true; }
    const int32 Requested=LandingControls.IndexOfByKey(Target);
    if(Requested==INDEX_NONE) return false;
    if(Requested==CurrentFloor) { bDoorRequested=true; CloseRemaining=AutoCloseDelay; }
    else { Destination=Requested; Phase=ETripoElevatorPhase::ClosingForTravel; bDoorRequested=false; }
    return true;
}
void ATripoElevator::RestoreFloor(int32 SavedFloor)
{
    bRestoredBeforeBegin=true;
    CurrentFloor=FMath::Clamp(SavedFloor,0,1); Destination=CurrentFloor; DoorAlpha=0;
    Phase=ETripoElevatorPhase::Docked; CloseRemaining=0; bDoorRequested=false; CurrentSpeed=0;
    Cabin->SetRelativeLocation(CurrentFloor ? Stop1 : Stop0);
    if(auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) LastAction=R->GetActionSeconds();
    ApplyDoors();
}
bool ATripoElevator::AllStable(UWorld* World)
{
    if(!World) return false;
    for(TActorIterator<ATripoElevator> It(World);It;++It) if(!It->IsStable()) return false;
    return true;
}
void ATripoElevator::ApplyDoors()
{
    const FVector L=LeftSlideDirection.GetSafeNormal()*DoorTravel*DoorAlpha;
    const FVector R=RightSlideDirection.GetSafeNormal()*DoorTravel*DoorAlpha;
    LeftDoor->SetRelativeLocation(LeftClosedPosition+L); RightDoor->SetRelativeLocation(RightClosedPosition+R);
    for(int32 I=0;I<2;++I)
    {
        const FVector Stop=I ? Stop1 : Stop0;
        const bool bHere=Phase!=ETripoElevatorPhase::Moving && I==CurrentFloor;
        LandingDoors[I*2]->SetRelativeLocation(Stop+LeftClosedPosition+LandingDoorOffset+(I ? UpperDoorOffset : FVector::ZeroVector)+(bHere ? L : FVector::ZeroVector));
        LandingDoors[I*2+1]->SetRelativeLocation(Stop+RightClosedPosition+LandingDoorOffset+(I ? UpperDoorOffset : FVector::ZeroVector)+(bHere ? R : FVector::ZeroVector));
        LandingBarriers[I]->SetRelativeLocation(Stop+(LeftClosedPosition+RightClosedPosition)*.5+LandingDoorOffset);
        // Docked doors themselves block passage until open; do not enable a solid box on an occupant.
        LandingBarriers[I]->SetCollisionEnabled(bHere ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
    }
}
void ATripoElevator::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    UpdateControlAnchors();
    if(!HasAuthority() || !R) return;
    const double Now=R->GetActionSeconds(); const float Dt=FMath::Max(0.,Now-LastAction); LastAction=Now;
    if(R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running) return;
    if(Phase==ETripoElevatorPhase::Moving)
    {
        const FVector Goal=Destination ? Stop1 : Stop0;
        const float Distance=FVector::Distance(Cabin->GetRelativeLocation(),Goal);
        const float DesiredSpeed=FMath::Min(FMath::Max(1.f,TravelSpeed),FMath::Sqrt(2*FMath::Max(1.f,TravelAcceleration)*Distance));
        CurrentSpeed=FMath::FInterpConstantTo(CurrentSpeed,DesiredSpeed,Dt,FMath::Max(1.f,TravelAcceleration));
        Cabin->SetRelativeLocation(FMath::VInterpConstantTo(Cabin->GetRelativeLocation(),Goal,Dt,FMath::Max(1.f,CurrentSpeed)));
        if(Cabin->GetRelativeLocation().Equals(Goal,.01)) { CurrentFloor=Destination; CurrentSpeed=0; Phase=ETripoElevatorPhase::Docked; bDoorRequested=true; CloseRemaining=FMath::Max(5.f,AutoCloseDelay); }
    }
    else
    {
        const bool bOccupied=DoorOccupied();
        if(bOccupied && (DoorAlpha>0 || bDoorRequested || !bManualDoors || Phase==ETripoElevatorPhase::ClosingForTravel)) CloseRemaining=AutoCloseDelay;
        else if(DoorAlpha>=1.f || !bDoorRequested) CloseRemaining=FMath::Max(0.f,CloseRemaining-Dt);
        const bool bOpen=(bOccupied && (DoorAlpha>0 || bDoorRequested || !bManualDoors || Phase==ETripoElevatorPhase::ClosingForTravel)) ||
            (Phase==ETripoElevatorPhase::Docked && CloseRemaining>0);
        if(!bOpen && DoorAlpha<=0) bDoorRequested=false;
        DoorAlpha=FMath::FInterpConstantTo(DoorAlpha,bOpen ? 1.f : 0.f,Dt,1.f/FMath::Max(.05f,DoorSeconds));
        if(Phase==ETripoElevatorPhase::ClosingForTravel && !bOccupied && DoorAlpha<=0) Phase=ETripoElevatorPhase::Moving;
    }
    ApplyDoors();
}

void ATripoElevator::UpdateControlAnchors()
{
    for(int32 I=0;I<LandingControls.Num();++I)
        if(LandingButtonActors.IsValidIndex(I) && IsValid(LandingButtonActors[I]) && LandingButtonOffsets.IsValidIndex(I))
            if(auto* Mount=LandingControls[I]->GetAttachParent())
                Mount->SetWorldLocation(LandingButtonActors[I]->GetActorTransform().TransformPosition(LandingButtonOffsets[I]));
}
