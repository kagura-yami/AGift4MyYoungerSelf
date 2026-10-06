#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoFloatingPlatform.generated.h"
class UBoxComponent;
class UStaticMeshComponent;
// Single-player, deterministic buoyancy; the moving collision base carries the character.
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoFloatingPlatform : public AActor
{
    GENERATED_BODY()
public:
    ATripoFloatingPlatform();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UBoxComponent> Deck;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Visual;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Buoyancy") float BobHeight=1.2f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Buoyancy") float SinkDepth=3.f;
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Buoyancy") float MaxTilt=3.f;
    UFUNCTION(BlueprintCallable) void ResetBuoyancy();
private:
    FTransform Rest;
    FVector Offset=FVector::ZeroVector, Speed=FVector::ZeroVector;
    int32 PreviousLoadCount=0;
    double LastRippleTime=-10;
    FVector LastRippleLoad=FVector::ZeroVector;
};
