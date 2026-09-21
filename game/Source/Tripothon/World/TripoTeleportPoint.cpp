#include "World/TripoTeleportPoint.h"
#include "Components/ArrowComponent.h"
#include "EngineUtils.h"
#include "Player/TripoCharacter.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
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
    if (!IsValid(Player) || !IsConfigured() || !GetWorld()) return false;
    if (FVector::DistSquared(Player->GetActorLocation(), GetActorLocation()) > FMath::Square(TriggerRadius)) return false;
    const double Now = GetWorld()->GetTimeSeconds(); if (Now < NextAllowedTime) return false;
    if (auto* Runtime = UTripoRuntimeSubsystem::GetRuntime(this))
        if (Runtime->IsActionPaused() || Runtime->GetRestorePhase() != ETripoRestorePhase::Running) return false;
    ATripoTeleportPoint* Destination = nullptr;
    for (TActorIterator<ATripoTeleportPoint> It(GetWorld()); It; ++It)
        if (*It != this && It->IsConfigured() && It->LinkId == LinkId) { Destination = *It; break; }
    if (!IsValid(Destination)) return false;
    if (!Player->TeleportTo(Destination->GetActorLocation() + FVector(0, 0, 90), Destination->GetActorRotation(), false, false)) return false;
    Player->GetCharacterMovement()->StopMovementImmediately(); NextAllowedTime = Now + Cooldown; Destination->NextAllowedTime = Now + Cooldown;
    return true;
}
