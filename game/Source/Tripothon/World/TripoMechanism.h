#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoMechanism.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UTripoIdentityComponent;
class UTripoHistoryComponent;
UENUM(BlueprintType)
enum class ETripoMechanismKind : uint8 { Plate, Switch, Gate, Platform };
USTRUCT(BlueprintType)
struct FTripoMechanismState
{
    GENERATED_BODY()
    UPROPERTY() FTransform Transform;
    UPROPERTY() bool bLatched = false;
    UPROPERTY() double Phase = 0;
    UPROPERTY() bool bCollisionEnabled = true;
};

// A configurable graph node. Actor references are level-local; saved snapshots use stable IDs.
UCLASS()
class TRIPOTHON_API ATripoMechanism : public AActor
{
    GENERATED_BODY()
public:
    ATripoMechanism();
    virtual void BeginPlay() override;
    virtual void Tick(float Delta) override;
    virtual void OnConstruction(const FTransform& Transform) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UTripoIdentityComponent> Identity;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTripoHistoryComponent> History;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Body;
    // Replace this mesh for art; Body remains the fixed gameplay collision shape.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Visual;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UBoxComponent> Sensor;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ETripoMechanismKind Kind = ETripoMechanismKind::Plate;
    UPROPERTY(EditAnywhere) bool bAllowEcho = false;
    UPROPERTY(EditAnywhere) bool bRequireAll = true;
    UPROPERTY(EditAnywhere) TArray<TObjectPtr<ATripoMechanism>> Inputs;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> RequiredEvents;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> RequiredChallenges;
    UPROPERTY(EditAnywhere) FVector Travel = FVector(0, 0, 300);
    UPROPERTY(EditAnywhere, meta=(ClampMin="0.1")) float Period = 6;
    UPROPERTY(EditAnywhere) bool bTimeAffectable = true;
    UFUNCTION(BlueprintPure) bool IsPowered() const;
    UFUNCTION(BlueprintCallable) bool Interact(AActor* Source);
    UFUNCTION(BlueprintCallable) void RefreshOccupants();
    void RefreshGate();
    FTripoMechanismState Capture() const;
    void Restore(const FTripoMechanismState& State);
    UFUNCTION(BlueprintCallable) FGuid AddSlow(UObject* Source, float Rate, double Duration);
    UFUNCTION(BlueprintCallable) void RemoveSlow(FGuid Handle);
    UFUNCTION(BlueprintPure) float GetLocalRate() const;
    bool ApplyRewindState(const FTripoMechanismState& State);
    bool bRewinding = false;
private:
    bool Evaluate(TSet<const ATripoMechanism*>& Visiting) const;
    bool bLatched = false;
    double Phase = 0;
    double LastAction = 0;
    FVector Origin;
    TSet<TWeakObjectPtr<AActor>> Occupants;
    double IntegrateLocalTime(double From, double To) const;
    struct FSlowSource { TWeakObjectPtr<UObject> Source; float Rate; double StartsAt; double EndsAt; };
    TMap<FGuid, FSlowSource> SlowSources;
};
