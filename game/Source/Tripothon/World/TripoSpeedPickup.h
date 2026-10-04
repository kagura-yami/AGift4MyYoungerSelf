#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoSpeedPickup.generated.h"
class USphereComponent;
UCLASS()
class TRIPOTHON_API ATripoSpeedPickup : public AActor
{
    GENERATED_BODY()
public:
    ATripoSpeedPickup();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USphereComponent> Volume;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<AActor> VisualActor;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Multiplier = 1.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Duration = 6.f;
    UPROPERTY(Transient, BlueprintReadOnly) bool bCollected = false;
private:
    int64 RestoreEpoch = 0;
};
