#include "World/TripoChaser.h"
#include "World/TripoChaseNavigation.h"
#include "World/TripoChaseHideZone.h"
#include "World/TripoWorldSubsystem.h"
#include "World/TripoInteractorComponent.h"
#include "Player/TripoCharacter.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

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
bool ATripoChaser::StartChase(ATripoCharacter* Player)
{
    if (!HasAuthority() || !IsValid(Player) || !Player->IsPlayerControlled()) return false;
    if (!GetController()) SpawnDefaultController();
    if (!Cast<AAIController>(GetController())) return false;
    Target = Player;
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
    UpdateLabel();
}
float ATripoChaser::CalculateSpeed(float WalkSpeed,float Fraction,bool bAirborne,bool bChasing,float ChaseRatio,float AggroBonus,float AirRatio,float SearchRatio)
{
    const float Ratio = bChasing ? FMath::Max(0.f,ChaseRatio)+FMath::Clamp(Fraction,0.f,1.f)*FMath::Max(0.f,AggroBonus) : FMath::Max(0.f,SearchRatio);
    return FMath::Max(0.f,WalkSpeed)*Ratio*(bChasing && bAirborne ? FMath::Max(0.f,AirRatio) : 1.f);
}
bool ATripoChaser::CanSee(ATripoCharacter* Player) const
{
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
    bNavigationBlocked = AI->MoveToLocation(Ground.Location,15.f,false,true,false,false,UTripoChaserNavFilter::StaticClass(),false)==EPathFollowingRequestResult::Failed;
    return !bNavigationBlocked;
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
    auto* Player = Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    const bool bTargetChanged = Target.Get()!=Player;
    Target = Player;
    if (!IsValid(Player) || Player->Interactor->bSuppressed) { StopChase(); return; }
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
        LastSeen = Player->GetActorLocation();
        SearchOrigin = LastSeen;
        Aggro = FMath::Min(FMath::Max(1.f,MaxAggro),Aggro+FMath::Max(0.f,SightAggroPerSecond)*Dt);
        State = ETripoChaseState::Chasing;
    }
    else
    {
        Aggro = bInHideZone && HideRate==0 ? 0 : FMath::Max(0.f,Aggro-(bInHideZone ? HideRate : FMath::Max(0.f,LostAggroPerSecond))*Dt);
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
        bVisible && Player->GetCharacterMovement()->IsFalling(),State==ETripoChaseState::Chasing,
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
    if(State==ETripoChaseState::Wandering && PatrolPoints.Num()>0) { TickPatrol(Dt); UpdateLabel(); return; }
    if (RepathRemaining<=0)
    {
        RepathRemaining = .3f;
        if (State==ETripoChaseState::Chasing) MoveToPoint(LastSeen);
        else if (State==ETripoChaseState::Searching && FVector::DistSquared2D(GetActorLocation(),LastSeen)>FMath::Square(100.f) && !bNavigationBlocked) MoveToPoint(LastSeen);
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
