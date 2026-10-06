#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoWaterHazard.generated.h"
class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
// A local water surface: feet entering the volume restore the current temporary checkpoint.
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoWaterHazard : public AActor
{
    GENERATED_BODY()
public:
    ATripoWaterHazard();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UBoxComponent> WaterVolume;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Surface;
    // Single-player visual impulse. Position is world-space; only this pool is affected.
    UFUNCTION(BlueprintCallable,Category="Water") void AddRipple(FVector Position, float Strength=1.f);
private:
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> WaterMaterial;
    double LastDeath=-10;
    int32 NextRipple=0;
};
