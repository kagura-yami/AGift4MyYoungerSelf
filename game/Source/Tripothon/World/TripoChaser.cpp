#include "World/TripoChaser.h"
#include "World/TripoChaseNavigation.h"
#include "World/TripoChaseHideZone.h"
#include "World/TripoWorldSubsystem.h"
#include "World/TripoInteractorComponent.h"
#include "Player/TripoCharacter.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
bool IsJumpableCart(const AActor* Actor)
{
    if (!Actor) return false;
    if (Actor->ActorHasTag(TEXT("ChaseJumpCart"))) return true;
    TInlineComponentArray<UMeshComponent*> Meshes(Actor);
    for (auto* Mesh:Meshes)
    {
        const UObject* Asset=nullptr;
        if (auto* Static=Cast<UStaticMeshComponent>(Mesh)) Asset=Static->GetStaticMesh();
        if (auto* Skeletal=Cast<USkeletalMeshComponent>(Mesh)) Asset=Skeletal->GetSkeletalMeshAsset();
        if (Asset && Asset->GetName().StartsWith(TEXT("SM_Prop_Cart"))) return true;
    }
    return false;
}
}
ATripoChaser::ATripoChaser()
{
    PrimaryActorTick.bCanEverTick = true;
    AIControllerClass = AAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    GetCapsuleComponent()->InitCapsuleSize(34,88);
    GetCapsuleComponent()->SetCollisionObjectType(ECC_GameTraceChannel1);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0,540,0);
    GetCharacterMovement()->MaxWalkSpeed = 300;
    bUseControllerRotationYaw = false;
    WhiteboxBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WhiteboxBody"));
    WhiteboxBody->SetupAttachment(RootComponent);
    WhiteboxBody->SetRelativeScale3D(FVector(.65,.65,1.7));
    WhiteboxBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WhiteboxBody->SetCanEverAffectNavigation(false);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) WhiteboxBody->SetStaticMesh(Cube.Object);
    DebugLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugLabel"));
    DebugLabel->SetupAttachment(RootComponent);
    DebugLabel->SetRelativeLocation(FVector(0,0,130));
    DebugLabel->SetWorldSize(26);
}
void ATripoChaser::BeginPlay()
{
    Super::BeginPlay(); InitialTransform=GetActorTransform();
    PatrolPoints.RemoveAll([](const TObjectPtr<AActor>& Point){return !IsValid(Point);});
    ResetAfterRestore();
}
void ATripoChaser::ResetAfterRestore()
{
    StopChase();
    if(!HasActorBegunPlay()) InitialTransform=GetActorTransform();
    const FTransform Home=PatrolPoints.Num()>0 && IsValid(PatrolPoints[0]) ? PatrolPoints[0]->GetActorTransform() : InitialTransform;
    TeleportTo(Home.GetLocation(),Home.Rotator(),false,false);
    PatrolIndex=0; PatrolDirection=1; PatrolWaitRemaining=0; bWaitingAtPoint=false;
    RepathRemaining=WanderRemaining=0; GraceRemaining=FMath::Max(0.f,SpawnGraceSeconds);
    JumpCooldown=0; bApproachingCart=false;
    SearchOrigin=LastSeen=GetActorLocation();
    if(auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) StartEpoch=R->GetEpoch();
    if(bStartPatrolling) { State=ETripoChaseState::Wandering; if(!GetController()) SpawnDefaultController(); }
}
void ATripoChaser::SelectNearestPatrolPoint()
{
    double Best=TNumericLimits<double>::Max();
    for(int32 I=0;I<PatrolPoints.Num();++I) if(IsValid(PatrolPoints[I]))
    { double D=FVector::DistSquared(GetActorLocation(),PatrolPoints[I]->GetActorLocation()); if(D<Best) { Best=D; PatrolIndex=I; } }
    bWaitingAtPoint=false; PatrolWaitRemaining=0;
}
void ATripoChaser::TickPatrol(float Dt)
{
    if(!PatrolPoints.IsValidIndex(PatrolIndex) || !IsValid(PatrolPoints[PatrolIndex])) { bNavigationBlocked=true; return; }
    if(FVector::DistSquared2D(GetActorLocation(),PatrolPoints[PatrolIndex]->GetActorLocation())<=FMath::Square(50.f))
    {
        if(auto* AI=Cast<AAIController>(GetController())) AI->StopMovement();
        if(!bWaitingAtPoint) { bWaitingAtPoint=true; PatrolWaitRemaining=FMath::Max(0.f,PatrolWaitSeconds); }
        PatrolWaitRemaining-=Dt;
        if(PatrolWaitRemaining<=0 && PatrolPoints.Num()>1)
        {
            if(bPingPongPatrol) { if(PatrolIndex+PatrolDirection>=PatrolPoints.Num() || PatrolIndex+PatrolDirection<0) PatrolDirection*=-1; PatrolIndex+=PatrolDirection; }
            else PatrolIndex=(PatrolIndex+1)%PatrolPoints.Num();
            bWaitingAtPoint=false; RepathRemaining=0;
        }
    }
    else if(RepathRemaining<=0) { RepathRemaining=.3f; MoveToPoint(PatrolPoints[PatrolIndex]->GetActorLocation()); }
}
bool ATripoChaser::IsInsidePatrolArea(const FVector& Point) const
{
    if (!bRestrictToPatrolArea) return true;
    const FVector D=(Point-PatrolAreaCenter).GetAbs();
    return D.X<=PatrolAreaExtent.X && D.Y<=PatrolAreaExtent.Y && D.Z<=PatrolAreaExtent.Z;
}
bool ATripoChaser::StartChase(ATripoCharacter* Player)
{
    if (!HasAuthority() || !IsValid(Player) || !Player->IsPlayerControlled() || !IsInsidePatrolArea(Player->GetActorLocation())) return false;
    if (!GetController()) SpawnDefaultController();
    if (!Cast<AAIController>(GetController())) return false;
    Target = Player;
    PursuitMemoryRemaining=FMath::Max(0.f,PursuitMemorySeconds);
    LastSeenDirection=Player->GetVelocity().GetSafeNormal2D();
    bCheckedEscapeDirection=false;
    BaseWalkSpeed = FMath::Max(1.f,Player->GetCharacterMovement()->MaxWalkSpeed);
    LastSeen = Player->GetActorLocation();
    SearchOrigin = LastSeen;
    Aggro = FMath::Clamp(InitialAggro,0.f,FMath::Max(1.f,MaxAggro));
    State = ETripoChaseState::Chasing;
    GraceRemaining = FMath::Max(0.f,SpawnGraceSeconds);
    RepathRemaining = WanderRemaining = ContactSeconds = 0;
    bNavigationBlocked = false;
    if (auto* R = UTripoRuntimeSubsystem::GetRuntime(this)) StartEpoch = R->GetEpoch();
    UpdateLabel();
    return true;
}
void ATripoChaser::StopChase()
{
    if (auto* AI = Cast<AAIController>(GetController())) AI->StopMovement();
    GetCharacterMovement()->StopMovementImmediately();
    State = ETripoChaseState::Dormant;
    Aggro = ContactSeconds = 0;
    Target.Reset();
    bApproachingCart=false;
    PursuitMemoryRemaining=0;
    bCheckedEscapeDirection=false;
    UpdateLabel();
}
float ATripoChaser::CalculateSpeed(float WalkSpeed,float Fraction,bool bAirborne,bool bChasing,float ChaseRatio,float AggroBonus,float AirRatio,float SearchRatio)
{
    const float Ratio = bChasing ? FMath::Max(0.f,ChaseRatio)+FMath::Clamp(Fraction,0.f,1.f)*FMath::Max(0.f,AggroBonus) : FMath::Max(0.f,SearchRatio);
    return FMath::Max(0.f,WalkSpeed)*Ratio*(bChasing && bAirborne ? FMath::Max(0.f,AirRatio) : 1.f);
}
bool ATripoChaser::CanSee(ATripoCharacter* Player) const
{
    if (!IsInsidePatrolArea(Player->GetActorLocation())) return false;
    const FVector Delta = Player->GetActorLocation()-GetActorLocation();
    if (Delta.SizeSquared()>FMath::Square(FMath::Max(0.f,SightRadius))) return false;
    if (Delta.SizeSquared2D()>FMath::Square(FMath::Max(0.f,CloseDetectionRadius)) &&
        FVector::DotProduct(GetActorForwardVector(),Delta.GetSafeNormal2D())<FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(SightHalfAngle,0.f,180.f)))) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ChaseSight),false,this);
    Params.AddIgnoredActor(Player);
    return !GetWorld()->LineTraceTestByChannel(GetActorLocation()+FVector(0,0,45),Player->GetActorLocation()+FVector(0,0,45),ECC_Visibility,Params);
}
bool ATripoChaser::HasClearContact(ATripoCharacter* Player) const
{
    const auto* Mine = GetCapsuleComponent();
    const auto* Other = Player->GetCapsuleComponent();
    if (!IsInsidePatrolArea(Player->GetActorLocation())) return false;
    const FVector Delta = Player->GetActorLocation()-GetActorLocation();
    // Jumping over the pursuer should not count as a catch.
    if (FMath::Abs(Delta.Z)>FMath::Min(Mine->GetScaledCapsuleHalfHeight(),Other->GetScaledCapsuleHalfHeight())) return false;
    if (Delta.SizeSquared2D()>FMath::Square(Mine->GetScaledCapsuleRadius()+Other->GetScaledCapsuleRadius()+FMath::Max(0.f,CatchSurfaceDistance))) return false;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ChaseContact),false,this);
    Params.AddIgnoredActor(Player);
    return !GetWorld()->LineTraceTestByChannel(GetActorLocation(),Player->GetActorLocation(),ECC_Visibility,Params);
}
bool ATripoChaser::MoveToPoint(const FVector& Point)
{
    auto* AI = Cast<AAIController>(GetController());
    if (!AI) return false;
    auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation Ground;
    if (!Nav || !Nav->ProjectPointToNavigation(Point,Ground,FVector(100,100,1000)))
    {
        AI->StopMovement();
        bNavigationBlocked = true;
        return false;
    }
    for(TActorIterator<ATripoChaseHideZone> It(GetWorld());It;++It) if(It->ContainsPoint(Ground.Location+FVector(0,0,88))) { AI->StopMovement(); bNavigationBlocked=true; return false; }
    if (bRestrictToPatrolArea)
    {
        auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(this,GetActorLocation(),Ground.Location,AI,UTripoChaserNavFilter::StaticClass());
        if (!Path || Path->PathPoints.IsEmpty()) { AI->StopMovement(); bNavigationBlocked=true; return false; }
        for (const FVector& P:Path->PathPoints) if (!IsInsidePatrolArea(P))
        { AI->StopMovement(); bNavigationBlocked=true; return false; }
    }
    // Reach the edge of a disconnected low obstacle before evaluating a safe jump.
    bNavigationBlocked = AI->MoveToLocation(Ground.Location,bApproachingCart ? 3.f : 15.f,false,true,false,false,UTripoChaserNavFilter::StaticClass(),true)==EPathFollowingRequestResult::Failed;
    return !bNavigationBlocked;
}
bool ATripoChaser::TryJumpObstacle(const FVector& Goal)
{
    if(!bJumpObstacles || JumpCooldown>0 || !GetCharacterMovement()->IsMovingOnGround()) return false;
    const FVector Start=GetActorLocation();
    const FVector Direction=(Goal-Start).GetSafeNormal2D();
    if(Direction.IsNearlyZero() || FVector::DistSquared2D(Goal,Start)<FMath::Square(120.f)) return false;
    FCollisionQueryParams Q(SCENE_QUERY_STAT(ChaserJump),false,this);
    if(Target.IsValid()) Q.AddIgnoredActor(Target.Get());
    FHitResult Obstacle;
    const float Half=GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    // A low obstacle must actually be ahead. Do not hop on unobstructed ground.
    const FVector Shin=Start-FVector(0,0,Half-30.f);
    if(!GetWorld()->SweepSingleByChannel(Obstacle,Shin,Shin+Direction*115.f,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(22.f),Q)) return false;
    if (!IsJumpableCart(Obstacle.GetActor())) return false;
    auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if(!Nav) return false;
    const float Gravity=FMath::Abs(GetCharacterMovement()->GetGravityZ());
    const auto Shape=FCollisionShape::MakeCapsule(GetCapsuleComponent()->GetScaledCapsuleRadius()-2.f,Half-2.f);
    for(float Distance : {220.f,320.f,420.f})
    {
        FNavLocation Ground;
        if(!Nav->ProjectPointToNavigation(Start+Direction*Distance-FVector(0,0,Half),Ground,FVector(45,45,180))) continue;
        const FVector End=Ground.Location+FVector(0,0,Half+3);
        if(FMath::Abs(End.Z-Start.Z)>100 || FVector::DistSquared2D(End,Start)<FMath::Square(150.f)) continue;
        // The ground footprint may cross this cart only, never boxes beneath the arc.
        FCollisionQueryParams Footprint=Q; Footprint.AddIgnoredActor(Obstacle.GetActor());
        const FVector FlatEnd(End.X,End.Y,Start.Z+3.f);
        if (GetWorld()->SweepTestByChannel(Start+FVector(0,0,3),FlatEnd,FQuat::Identity,ECC_Visibility,Shape,Footprint)) continue;
        bool bSafe=IsInsidePatrolArea(End);
        for(TActorIterator<ATripoChaseHideZone> It(GetWorld());It;++It) if(It->ContainsPoint(End)) { bSafe=false; break; }
        if(!bSafe) continue;
        constexpr float Flight=1.2f;
        FVector Velocity=(End-Start)/Flight;
        Velocity.Z+=.5f*Gravity*Flight;
        FVector Previous=Start;
        for(int32 I=1;I<=18 && bSafe;++I)
        {
            const float T=Flight*I/18.f;
            const FVector Position=Start+Velocity*T-FVector(0,0,.5f*Gravity*T*T);
            if (!IsInsidePatrolArea(Position)) bSafe=false;
            FHitResult Hit;
            if(GetWorld()->SweepSingleByChannel(Hit,Previous,Position,FQuat::Identity,ECC_Visibility,Shape,Q)) bSafe=false;
            for(TActorIterator<ATripoChaseHideZone> It(GetWorld());It;++It) if(It->ContainsPoint(Position)) bSafe=false;
            Previous=Position;
        }
        if(!bSafe) continue;
        if(auto* AI=Cast<AAIController>(GetController())) AI->StopMovement();
        LaunchCharacter(Velocity,true,true);
        JumpCooldown=1.7f; RepathRemaining=Flight; bNavigationBlocked=false;
        return true;
    }
    JumpCooldown=.35f;
    return false;
}
bool ATripoChaser::FindCartApproach(const FVector& Goal)
{
    auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    auto* AI=Cast<AAIController>(GetController());
    if (!Nav || !AI) return false;
    const FVector Start=GetActorLocation();
    double Best=TNumericLimits<double>::Max();
    bool Found=false;
    for (TActorIterator<AActor> It(GetWorld());It;++It)
    {
        if (!IsJumpableCart(*It) || FVector::DistSquared2D(It->GetActorLocation(),Start)>FMath::Square(650.f)) continue;
        FVector Center,Extent; It->GetActorBounds(false,Center,Extent);
        for (const FVector& D:{FVector(1,0,0),FVector(-1,0,0),FVector(0,1,0),FVector(0,-1,0)})
        {
            const float Reach=FMath::Abs(D.X)*Extent.X+FMath::Abs(D.Y)*Extent.Y+100.f;
            FNavLocation Near,Far;
            FVector A=Center-D*Reach; A.Z=Start.Z-88;
            FVector B=Center+D*(Reach+30); B.Z=A.Z;
            if (!Nav->ProjectPointToNavigation(A,Near,FVector(25,25,75)) || !Nav->ProjectPointToNavigation(B,Far,FVector(25,25,75))) continue;
            if (!IsInsidePatrolArea(Near.Location) || !IsInsidePatrolArea(Far.Location)) continue;
            if (FVector::DistSquared2D(Far.Location,Goal)>=FVector::DistSquared2D(Near.Location,Goal)) continue;
            auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(this,Start,Near.Location,AI,UTripoChaserNavFilter::StaticClass());
            if (!Path || !Path->IsValid() || Path->IsPartial()) continue;
            bool Inside=true;
            for (const FVector& P:Path->PathPoints) if (!IsInsidePatrolArea(P)) Inside=false;
            if (!Inside) continue;
            const double Score=Path->GetPathLength()+FVector::Dist2D(Far.Location,Goal);
            if (Score<Best) { Best=Score; CartApproach=Near.Location+FVector(0,0,88); CartLanding=Far.Location+FVector(0,0,88); Found=true; }
        }
    }
    bApproachingCart=Found;
    return Found;
}
void ATripoChaser::Tick(float Dt)
{
    Super::Tick(Dt);
    if (!HasAuthority() || State==ETripoChaseState::Dormant || State==ETripoChaseState::Caught) return;
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (R && R->GetEpoch()!=StartEpoch) { ResetAfterRestore(); return; }
    if (R && (R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running))
    {
        if (auto* AI = Cast<AAIController>(GetController())) AI->StopMovement();
        GetCharacterMovement()->StopMovementImmediately();
        RepathRemaining = 0;
        return;
    }
    JumpCooldown=FMath::Max(0.f,JumpCooldown-Dt);
    auto* Player = Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    const bool bTargetChanged = Target.Get()!=Player;
    Target = Player;
    if (!IsValid(Player) || Player->Interactor->bSuppressed) { StopChase(); return; }
    // Leaving the corridor immediately ends pursuit, including contact through its doorway.
    if (bRestrictToPatrolArea && !IsInsidePatrolArea(Player->GetActorLocation())) Aggro=0;
    if (bRestrictToPatrolArea)
    {
        const float Radius=GetCapsuleComponent()->GetScaledCapsuleRadius();
        const FVector Margin(FMath::Max(0.f,PatrolAreaExtent.X-Radius),FMath::Max(0.f,PatrolAreaExtent.Y-Radius),PatrolAreaExtent.Z);
        const FVector P=GetActorLocation();
        const FVector Safe=PatrolAreaCenter+(P-PatrolAreaCenter).BoundToBox(-Margin,Margin);
        if (!P.Equals(Safe,.1f))
        {
            if (auto* AI=Cast<AAIController>(GetController())) AI->StopMovement();
            GetCharacterMovement()->StopMovementImmediately();
            SetActorLocation(Safe,true);
            RepathRemaining=0;
        }
    }
    float HideRate = -1;
    for (TActorIterator<ATripoChaseHideZone> It(GetWorld()); It; ++It)
        if (It->ContainsPoint(Player->GetActorLocation()))
        {
            const float Rate = FMath::Max(0.f,It->AggroReductionPerSecond);
            if (Rate==0) { HideRate=0; break; }
            HideRate = FMath::Max(HideRate,Rate);
        }
    const bool bInHideZone = HideRate>=0;
    const bool bVisible = !bInHideZone && CanSee(Player);
    const ETripoChaseState Previous = State;
    if (bVisible)
    {
        PursuitMemoryRemaining=FMath::Max(0.f,PursuitMemorySeconds);
        if (Player->GetVelocity().SizeSquared2D()>100.f) LastSeenDirection=Player->GetVelocity().GetSafeNormal2D();
        bCheckedEscapeDirection=false;
        LastSeen = Player->GetActorLocation();
        SearchOrigin = LastSeen;
        Aggro = FMath::Min(FMath::Max(1.f,MaxAggro),Aggro+FMath::Max(0.f,SightAggroPerSecond)*Dt);
        State = ETripoChaseState::Chasing;
    }
    else
    {
        if (bInHideZone || !IsInsidePatrolArea(Player->GetActorLocation())) PursuitMemoryRemaining=0;
        else PursuitMemoryRemaining=FMath::Max(0.f,PursuitMemoryRemaining-Dt);
        Aggro = bInHideZone && HideRate==0 ? 0 : FMath::Max(0.f,Aggro-(bInHideZone ? HideRate : FMath::Max(0.f,LostAggroPerSecond))*Dt);
        if (PursuitMemoryRemaining>0) Aggro=FMath::Max(Aggro,1.f);
        State = Aggro>0 ? ETripoChaseState::Searching : ETripoChaseState::Wandering;
    }
    if (State!=Previous || bTargetChanged)
    {
        if (auto* AI = Cast<AAIController>(GetController())) AI->StopMovement();
        RepathRemaining = WanderRemaining = ContactSeconds = 0;
        if(State==ETripoChaseState::Wandering) SelectNearestPatrolPoint();
    }
    // Hidden movement and a dash's instantaneous velocity must not leak into pursuit.
    if (bVisible) BaseWalkSpeed = FMath::Max(1.f,Player->GetCharacterMovement()->MaxWalkSpeed);
    const float Desired = CalculateSpeed(BaseWalkSpeed,Aggro/FMath::Max(1.f,MaxAggro),
        bVisible && Player->GetCharacterMovement()->IsFalling(),State==ETripoChaseState::Chasing || (State==ETripoChaseState::Searching && PursuitMemoryRemaining>0),
        ChaseSpeedRatio,FullAggroSpeedBonus,AirborneSpeedRatio,SearchSpeedRatio);
    GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpConstantTo(GetCharacterMovement()->MaxWalkSpeed,Desired,Dt,FMath::Max(1.f,SpeedChangePerSecond));
    GraceRemaining = FMath::Max(0.f,GraceRemaining-Dt);
    ContactSeconds = !bInHideZone && GraceRemaining<=0 && HasClearContact(Player) ? ContactSeconds+Dt : 0;
    if (ContactSeconds>=FMath::Max(.05f,CatchHoldSeconds))
    {
        StopChase();
        State = ETripoChaseState::Caught;
        const bool bRestored = UTripoWorldSubsystem::Get(this)->RestorePlayer(Player);
        UE_LOG(LogTemp,Display,TEXT("Chase caught %s; checkpoint restored=%d"),*GetNameSafe(Player),bRestored);
        OnPlayerCaught.Broadcast(Player,bRestored);
        UpdateLabel();
        return;
    }
    RepathRemaining -= Dt;
    WanderRemaining -= Dt;
    // Continue around the last observed escape direction; never read an unseen player's position.
    if (State==ETripoChaseState::Searching && PursuitMemoryRemaining>0 && !bCheckedEscapeDirection &&
        FVector::DistSquared2D(GetActorLocation(),LastSeen)<FMath::Square(100.f))
    {
        bCheckedEscapeDirection=true;
        if (auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
        {
            FVector Best=LastSeen; float BestScore=0;
            for (int32 I=0;I<24;++I)
            {
                FNavLocation Candidate;
                if (!Nav->GetRandomReachablePointInRadius(LastSeen,500.f,Candidate) || !IsInsidePatrolArea(Candidate.Location)) continue;
                const FVector Delta=Candidate.Location-LastSeen;
                const float Score=FVector::DotProduct(Delta.GetSafeNormal2D(),LastSeenDirection)*Delta.Size2D();
                if (Score>BestScore) { BestScore=Score; Best=Candidate.Location; }
            }
            if (BestScore>80) { LastSeen=Best; RepathRemaining=0; }
        }
    }
    const FVector Goal=State==ETripoChaseState::Wandering && PatrolPoints.IsValidIndex(PatrolIndex) && IsValid(PatrolPoints[PatrolIndex])
        ? PatrolPoints[PatrolIndex]->GetActorLocation() : LastSeen;
    if (bApproachingCart && !GetCharacterMovement()->IsFalling())
    {
        if (FVector::DistSquared2D(GetActorLocation(),CartApproach)<FMath::Square(8.f))
        {
            if (auto* AI=Cast<AAIController>(GetController())) AI->StopMovement();
            GetCharacterMovement()->StopMovementImmediately();
            if (TryJumpObstacle(CartLanding)) { bApproachingCart=false; UpdateLabel(); return; }
            UpdateLabel(); return;
        }
        else
        {
            if (RepathRemaining<=0) { RepathRemaining=.3f; MoveToPoint(CartApproach); }
            UpdateLabel(); return;
        }
    }
    if ((bNavigationBlocked || GetVelocity().Size2D()<35.f) && !bWaitingAtPoint)
    {
        if (TryJumpObstacle(Goal)) { UpdateLabel(); return; }
        if (RepathRemaining<=0 && FindCartApproach(Goal))
        { RepathRemaining=.3f; MoveToPoint(CartApproach); UpdateLabel(); return; }
    }
    if(GetCharacterMovement()->IsFalling()) { UpdateLabel(); return; }
    if(State==ETripoChaseState::Wandering && PatrolPoints.Num()>0) { TickPatrol(Dt); UpdateLabel(); return; }
    if (RepathRemaining<=0)
    {
        RepathRemaining = .3f;
        if (State==ETripoChaseState::Chasing) MoveToPoint(LastSeen);
        else if (State==ETripoChaseState::Searching && FVector::DistSquared2D(GetActorLocation(),LastSeen)>FMath::Square(100.f)) MoveToPoint(LastSeen);
        else if (WanderRemaining<=0)
        {
            WanderRemaining = FMath::Max(.2f,WanderInterval);
            auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
            FNavLocation Point;
            if (Nav && Nav->GetRandomReachablePointInRadius(GetActorLocation(),FMath::Max(100.f,WanderRadius),Point))
            {
                if (FVector::DistSquared2D(Point.Location,SearchOrigin)<=FMath::Square(FMath::Max(100.f,WanderRadius)*2)) MoveToPoint(Point.Location);
                else MoveToPoint(SearchOrigin);
            }
            else bNavigationBlocked = true;
        }
    }
    UpdateLabel();
}
void ATripoChaser::UpdateLabel()
{
    DebugLabel->SetVisibility(bShowDebug);
    if (!bShowDebug) return;
    const FString Name = StaticEnum<ETripoChaseState>()->GetNameStringByValue(static_cast<int64>(State));
    DebugLabel->SetText(FText::FromString(FString::Printf(TEXT("%s | %.0f / %.0f%s"),*Name,Aggro,MaxAggro,bNavigationBlocked ? TEXT(" | NO PATH") : TEXT(""))));
    DebugLabel->SetTextRenderColor(State==ETripoChaseState::Chasing ? FColor::Red : FColor::Yellow);
}
