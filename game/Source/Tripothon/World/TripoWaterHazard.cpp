#include "World/TripoWaterHazard.h"
#include "World/TripoWorldSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
ATripoWaterHazard::ATripoWaterHazard()
{
    PrimaryActorTick.bCanEverTick=true;
    WaterVolume=CreateDefaultSubobject<UBoxComponent>(TEXT("WaterVolume")); SetRootComponent(WaterVolume);
    WaterVolume->SetBoxExtent(FVector(500,500,100)); WaterVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Surface=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Surface")); Surface->SetupAttachment(WaterVolume);
    Surface->SetCollisionEnabled(ECollisionEnabled::NoCollision); Surface->SetCastShadow(false);
}
void ATripoWaterHazard::BeginPlay()
{
    Super::BeginPlay(); WaterMaterial=Surface->CreateDynamicMaterialInstance(0);
}
void ATripoWaterHazard::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running) return;
    const double Now=R->GetActionSeconds();
    if (WaterMaterial) WaterMaterial->SetScalarParameterValue(TEXT("WaterTime"),Now);
    auto* P=Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if (!P || P->bDebugFlying || Now-LastDeath<.4) return;
    const FVector Feet=P->GetActorLocation()-FVector(0,0,P->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    const FVector Local=WaterVolume->GetComponentTransform().InverseTransformPosition(Feet);
    const FVector E=WaterVolume->GetUnscaledBoxExtent();
    if (FMath::Abs(Local.X)>E.X || FMath::Abs(Local.Y)>E.Y || Local.Z>E.Z || Local.Z< -E.Z) return;
    // The surface has no collision; platforms and dry floors protect feet that are above it.
    if (UTripoWorldSubsystem::Get(this)->RestorePlayer(P)) LastDeath=Now;
}

void ATripoWaterHazard::AddRipple(FVector Position, float Strength)
{
    auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if (!WaterMaterial || !R || R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running) return;
    const FVector Local=WaterVolume->GetComponentTransform().InverseTransformPosition(Position);
    const FVector E=WaterVolume->GetUnscaledBoxExtent();
    if (FMath::Abs(Local.X)>E.X || FMath::Abs(Local.Y)>E.Y || FMath::Abs(Local.Z-E.Z)>80.f) return;
    const FName Name(*FString::Printf(TEXT("Ripple%d"),NextRipple));
    WaterMaterial->SetVectorParameterValue(Name,FLinearColor(Position.X,Position.Y,R->GetActionSeconds(),FMath::Clamp(Strength,0.f,1.f)));
    NextRipple=(NextRipple+1)%8;
}

