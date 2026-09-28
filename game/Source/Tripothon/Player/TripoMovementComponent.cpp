#include "Player/TripoMovementComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
ETripoAbilityFailure UTripoMovementComponent::CanBurst(bool bUp) const
{
    if (!CharacterOwner || (!IsMovingOnGround() && !IsFalling())) return ETripoAbilityFailure::Blocked;
    return (bUp ? bUpSpent : bDashSpent) ? ETripoAbilityFailure::AirUseSpent : ETripoAbilityFailure::None;
}
bool UTripoMovementComponent::BeginBurst(bool bUp, FVector Direction, float Magnitude, float Duration)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || CanBurst(bUp) != ETripoAbilityFailure::None || Duration <= 0) return false;
    BurstHit = FHitResult(); bGroundBurst = IsMovingOnGround() && !bUp;
    BurstDirection = bUp ? FVector::UpVector : Direction.GetSafeNormal2D();
    BurstSpeed = bUp ? Magnitude : Magnitude / Duration;
    if (bUp)
    {
        // Total rise is v*t + v*v/(2*g). Solve for the new speed so both the
        // powered rise and subsequent ballistic rise contribute to the target.
        const float HeightScale = FMath::Clamp(UpDashHeightScale, 0.f, 1.f);
        const float Gravity = -GetGravityZ();
        const float GravityTime = Gravity * Duration;
        BurstSpeed = Gravity > UE_SMALL_NUMBER
            ? FMath::Sqrt(FMath::Square(GravityTime) + HeightScale *
                (FMath::Square(Magnitude) + 2.f * GravityTime * Magnitude)) - GravityTime
            : Magnitude * HeightScale;
    }
    // Normalize only the horizontal input so diagonals
    // travel the same distance. Ordinary dash continues to use its own direction.
    BurstHorizontalVelocity = bUp
        ? Direction.GetSafeNormal2D() * (FMath::Max(0.f, UpDashHorizontalDistance) / Duration)
        : FVector::ZeroVector;
    LastAction = R->GetActionSeconds(); BurstEnd = LastAction + Duration;
    if (bUp) bUpSpent = true; else bDashSpent = true;
    SetMovementMode(MOVE_Custom, 1); return true;
}
void UTripoMovementComponent::EndBurst()
{
    if (!IsBursting()) return;
    SetMovementMode(MOVE_Falling);
    Velocity.X = FMath::Clamp(Velocity.X, -MaxWalkSpeed, MaxWalkSpeed);
    Velocity.Y = FMath::Clamp(Velocity.Y, -MaxWalkSpeed, MaxWalkSpeed);
}
void UTripoMovementComponent::ResetAirUses() { bDashSpent = bUpSpent = false; LastWall.Reset(); }
void UTripoMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
    if (MovementMode == MOVE_Custom && CustomMovementMode == 2) return;
    if (!IsBursting()) { Super::PhysCustom(DeltaTime, Iterations); return; }
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this);
    if (!R || !UpdatedComponent) { EndBurst(); return; }
    const double Now = FMath::Min(R->GetActionSeconds(), BurstEnd);
    const float Step = float(FMath::Max(0., Now - LastAction)); LastAction = Now;
    FVector Move = (BurstDirection * BurstSpeed + BurstHorizontalVelocity) * Step;
    if (BurstDirection.Z == 0 && !bGroundBurst) { Velocity.Z += GetGravityZ() * Step; Move.Z = Velocity.Z * Step; }
    FHitResult Hit;
    SafeMoveUpdatedComponent(Move, UpdatedComponent->GetComponentQuat(), true, Hit);
    if (Step > 0) { const float Z = Velocity.Z; Velocity = BurstDirection * BurstSpeed + BurstHorizontalVelocity; if (BurstDirection.Z == 0) Velocity.Z = bGroundBurst ? 0 : Z; }
    if (Hit.bBlockingHit)
    {
        BurstHit = Hit;
        if (Hit.ImpactNormal.Z > .7 && BurstDirection.Z == 0) SlideAlongSurface(Move, 1-Hit.Time, Hit.Normal, Hit, true);
        else { if (BurstDirection.Z > 0) Velocity.Z = 0; EndBurst(); return; }
    }
    if (Now >= BurstEnd) EndBurst();
}
bool UTripoMovementComponent::ReplayTo(const FTransform& Transform, const FVector& RecordedVelocity)
{
    if (MovementMode != MOVE_Custom || CustomMovementMode != 2 || !UpdatedComponent) return false;
    FHitResult Hit; SafeMoveUpdatedComponent(Transform.GetLocation() - UpdatedComponent->GetComponentLocation(), Transform.GetRotation(), true, Hit);
    if (Hit.bBlockingHit) { Velocity = FVector::ZeroVector; return false; }
    Velocity = RecordedVelocity; return true;
}
ETripoAbilityFailure UTripoMovementComponent::WallJump(float Strength, float Reach)
{
    if (!IsFalling() || !CharacterOwner) return ETripoAbilityFailure::Blocked;
    FHitResult Hit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoWallJump), false, CharacterOwner);
    const FVector Start = CharacterOwner->GetActorLocation();
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + CharacterOwner->GetActorForwardVector() * (Reach + 34), ECC_Visibility, Query) || !Hit.GetActor() || !Hit.GetActor()->ActorHasTag(TEXT("TripoWallJump")) || FMath::Abs(Hit.ImpactNormal.Z) > .25f) return ETripoAbilityFailure::NoTarget;
    if (LastWall == Hit.GetComponent() && FVector::DistSquared(Start, LastWallPoint) < FMath::Square(130.)) return ETripoAbilityFailure::AirUseSpent;
    LastWall = Hit.GetComponent(); LastWallPoint = Start;
    Velocity = Hit.ImpactNormal * (Strength * .7f) + FVector(0,0,Strength);
    return ETripoAbilityFailure::None;
}
