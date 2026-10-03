#include "World/TripoTeleportPoint.h"
#include "World/TripoElevator.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "EngineUtils.h"
#include "Player/TripoCharacter.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/TripoMovementComponent.h"
#include "Kismet/GameplayStatics.h"
ATripoTeleportPoint::ATripoTeleportPoint()
{
    PrimaryActorTick.bCanEverTick = false;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Direction = CreateDefaultSubobject<UArrowComponent>(TEXT("Direction"));
    Direction->SetupAttachment(RootComponent); Direction->ArrowSize = 1.5f; Direction->SetHiddenInGame(true);
}
bool ATripoTeleportPoint::IsConfigured() const { return !LinkId.IsNone(); }
bool ATripoTeleportPoint::TryTeleport(ATripoCharacter* Player)
{
    if (!bEntryEnabled || !IsValid(Player) || !IsConfigured() || !GetWorld()) return false;
    if (FVector::DistSquared(Player->GetActorLocation(), GetActorLocation()) > FMath::Square(TriggerRadius)) return false;
    if(bRequireForwardDirection && FVector::DotProduct(Player->GetActorForwardVector().GetSafeNormal2D(),GetActorForwardVector().GetSafeNormal2D())<MinimumForwardDot) return false;
    if(RequiredElevator && (!RequiredElevator->IsStable() || RequiredElevator->CurrentFloor!=RequiredFloor)) return false;
    const double Now = GetWorld()->GetTimeSeconds(); if (Now < NextAllowedTime) return false;
    if (auto* Runtime = UTripoRuntimeSubsystem::GetRuntime(this))
        if (Runtime->IsActionPaused() || Runtime->GetRestorePhase() != ETripoRestorePhase::Running) return false;
    ATripoTeleportPoint* Destination = nullptr;
    for (TActorIterator<ATripoTeleportPoint> It(GetWorld()); It; ++It)
        if (*It != this && It->IsConfigured() && It->LinkId == LinkId) { if (Destination) return false; Destination = *It; }
    if (!IsValid(Destination)) return false;
    // Skip the intervening door only. Never skip clearance at the destination.
    const FVector Goal=Destination->GetActorLocation()+Destination->DestinationOffset;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(DashPortalLanding),false,Player);
    const auto* Capsule=Player->GetCapsuleComponent();
    if(GetWorld()->OverlapBlockingTestByChannel(Goal,FQuat::Identity,ECC_Pawn,
        FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(),Capsule->GetScaledCapsuleHalfHeight()),Query)) return false;
    if(!Player->TeleportTo(Goal,Player->GetActorRotation(),false,true)) return false;
    CastChecked<UTripoMovementComponent>(Player->GetCharacterMovement())->EndBurst();
    Player->GetCharacterMovement()->StopMovementImmediately(); NextAllowedTime = Now + Cooldown; Destination->NextAllowedTime = Now + Cooldown;
    return true;
}
