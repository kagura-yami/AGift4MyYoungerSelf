#pragma once
#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/TripoAbilityDefinition.h"
#include "TripoMovementComponent.generated.h"
UCLASS()
class TRIPOTHON_API UTripoMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()
public:
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
    ETripoAbilityFailure CanBurst(bool bUp) const;
    bool BeginBurst(bool bUp, FVector Direction, float DistanceOrSpeed, float Duration);
    void EndBurst();
    void ResetAirUses();
    bool ReplayTo(const FTransform& Transform, const FVector& RecordedVelocity);
    ETripoAbilityFailure WallJump(float Strength, float Reach);
    bool IsBursting() const { return MovementMode == MOVE_Custom && CustomMovementMode == 1; }
    bool bDashSpent = false;
    bool bUpSpent = false;
    FHitResult BurstHit;
private:
    FVector BurstDirection;
    float BurstSpeed = 0;
    double BurstEnd = 0;
    double LastAction = 0;
    bool bGroundBurst = false;
    TWeakObjectPtr<UPrimitiveComponent> LastWall;
    FVector LastWallPoint;
};
