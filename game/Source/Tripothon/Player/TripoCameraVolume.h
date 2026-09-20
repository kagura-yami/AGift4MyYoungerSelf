#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoCameraVolume.generated.h"

class UBoxComponent;
class UTripoIdentityComponent;

UCLASS()
class TRIPOTHON_API ATripoCameraVolume : public AActor
{
    GENERATED_BODY()
public:
    ATripoCameraVolume();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Identity") TObjectPtr<UTripoIdentityComponent> Identity;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera") TObjectPtr<UBoxComponent> Bounds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera") float AnchorYaw = 90.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera") int32 Priority = 0;
};
