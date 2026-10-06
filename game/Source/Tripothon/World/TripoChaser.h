#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TripoChaser.generated.h"
class ATripoCharacter;
class UStaticMeshComponent;
class UTextRenderComponent;
UENUM(BlueprintType)
enum class ETripoChaseState : uint8 { Dormant, Chasing, Searching, Wandering, Caught };
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTripoChaseCaught, ATripoCharacter*, Player, bool, bRestored);

/** Navigation-driven single-player pursuer; never copies dash velocity or teleports through obstacles. */
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoChaser : public ACharacter
{
    GENERATED_BODY()
public:
    ATripoChaser();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UFUNCTION(BlueprintCallable, Category="Chase") void ResetAfterRestore();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Patrol") TArray<TObjectPtr<AActor>> PatrolPoints;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Patrol") bool bStartPatrolling = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Patrol") bool bPingPongPatrol = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Patrol", meta=(ClampMin="0")) float PatrolWaitSeconds = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Patrol") bool bRestrictToPatrolArea = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Patrol") FVector PatrolAreaCenter = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Patrol") FVector PatrolAreaExtent = FVector(200,1000,300);
    UFUNCTION(BlueprintPure, Category="Chase|Patrol") bool IsInsidePatrolArea(const FVector& Point) const;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Chase|Patrol") int32 PatrolIndex = 0;
    UFUNCTION(BlueprintCallable, Category="Chase") bool StartChase(ATripoCharacter* Player);
    UFUNCTION(BlueprintCallable, Category="Chase") void StopChase();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chase") TObjectPtr<UStaticMeshComponent> WhiteboxBody;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chase") TObjectPtr<UTextRenderComponent> DebugLabel;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Chase") ETripoChaseState State = ETripoChaseState::Dormant;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Chase") float Aggro = 0;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Chase") bool bNavigationBlocked = false;
    UPROPERTY(BlueprintAssignable, Category="Chase") FTripoChaseCaught OnPlayerCaught;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Debug") bool bShowDebug = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Aggro", meta=(ClampMin="1")) float MaxAggro = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Aggro", meta=(ClampMin="0")) float InitialAggro = 70;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Aggro", meta=(ClampMin="0")) float SightAggroPerSecond = 30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Aggro", meta=(ClampMin="0")) float LostAggroPerSecond = 12;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Sight", meta=(ClampMin="0")) float SightRadius = 1800;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Sight", meta=(ClampMin="0", ClampMax="180")) float SightHalfAngle = 70;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Sight", meta=(ClampMin="0")) float CloseDetectionRadius = 220;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Movement", meta=(ClampMin="0")) float ChaseSpeedRatio = .92f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Movement", meta=(ClampMin="0")) float FullAggroSpeedBonus = .08f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Movement", meta=(ClampMin="0")) float AirborneSpeedRatio = .85f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Movement", meta=(ClampMin="0")) float SearchSpeedRatio = .55f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Movement", meta=(ClampMin="0")) float SpeedChangePerSecond = 220;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Movement") bool bJumpObstacles = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Search", meta=(ClampMin="0")) float PursuitMemorySeconds = 8.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Search", meta=(ClampMin="100")) float WanderRadius = 650;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Search", meta=(ClampMin=".2")) float WanderInterval = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Catch", meta=(ClampMin="0")) float CatchSurfaceDistance = 12;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Catch", meta=(ClampMin=".05")) float CatchHoldSeconds = .15f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase|Catch", meta=(ClampMin="0")) float SpawnGraceSeconds = 1.2f;
    UFUNCTION(BlueprintPure, Category="Chase") static float CalculateSpeed(float WalkSpeed, float AggroFraction, bool bAirborne, bool bChasing,
        float ChaseRatio, float AggroBonus, float AirRatio, float SearchRatio);
private:
    UPROPERTY() TWeakObjectPtr<ATripoCharacter> Target;
    FVector LastSeen = FVector::ZeroVector;
    FVector SearchOrigin = FVector::ZeroVector;
    float BaseWalkSpeed = 540;
    float RepathRemaining = 0;
    float WanderRemaining = 0;
    float ContactSeconds = 0;
    float GraceRemaining = 0;
    int64 StartEpoch = 0;
    FTransform InitialTransform;
    int32 PatrolDirection = 1;
    float PatrolWaitRemaining = 0;
    bool bWaitingAtPoint = false;
    float JumpCooldown = 0.f;
    float PursuitMemoryRemaining = 0.f;
    FVector LastSeenDirection = FVector::ZeroVector;
    bool bCheckedEscapeDirection = false;
    bool TryJumpObstacle(const FVector& Goal);
    bool FindCartApproach(const FVector& Goal);
    FVector CartApproach = FVector::ZeroVector;
    FVector CartLanding = FVector::ZeroVector;
    bool bApproachingCart = false;
    void TickPatrol(float Dt);
    void SelectNearestPatrolPoint();
    bool CanSee(ATripoCharacter* Player) const;
    bool HasClearContact(ATripoCharacter* Player) const;
    bool MoveToPoint(const FVector& Point);
    void UpdateLabel();
};
