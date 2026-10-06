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
    virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
    ETripoAbilityFailure CanBurst(bool bUp) const;
    bool BeginBurst(bool bUp, FVector Direction, float DistanceOrSpeed, float Duration);
    void EndBurst();
    void ResetAirUses();
    bool ReplayTo(const FTransform& Transform, const FVector& RecordedVelocity);
    ETripoAbilityFailure WallJump(float Strength, float Reach);
    bool IsBursting() const { return MovementMode == MOVE_Custom && CustomMovementMode == 1; }
    // Horizontal travel during the powered phase of UpDash, in centimeters.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tripo|Movement", meta=(ClampMin="0"))
    float UpDashHorizontalDistance = 31.2f;
    // Scale the unobstructed rise, including momentum after the powered phase.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tripo|Movement", meta=(ClampMin="0", ClampMax="1"))
    float UpDashHeightScale = .715f;
    bool bDashSpent = false;
    bool bUpSpent = false;
    FHitResult BurstHit;
private:
    FVector BurstDirection;
    FVector BurstHorizontalVelocity = FVector::ZeroVector;
    float BurstSpeed = 0;
    double BurstEnd = 0;
    double LastAction = 0;
    bool bGroundBurst = false;
    TWeakObjectPtr<UPrimitiveComponent> LastWall;
    FVector LastWallPoint;
};
