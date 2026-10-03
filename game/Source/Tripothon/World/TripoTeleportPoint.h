#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoTeleportPoint.generated.h"
class UArrowComponent;
class ATripoElevator;
class ATripoCharacter;
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoTeleportPoint : public AActor
{
    GENERATED_BODY()
public:
    ATripoTeleportPoint();
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tripo|Teleport") TObjectPtr<UArrowComponent> Direction;
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Tripo|Teleport") FName LinkId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tripo|Teleport", meta=(ClampMin="1.0")) float TriggerRadius = 100.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tripo|Teleport", meta=(ClampMin="0.0")) float Cooldown = 0.35f;
    UFUNCTION(BlueprintPure, Category="Tripo|Teleport") bool IsConfigured() const;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tripo|Teleport", meta=(DeprecatedProperty, DeprecationMessage="Destination clearance is always checked.")) bool bIgnoreCollisionInLv4 = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tripo|Teleport") FVector DestinationOffset = FVector(0,0,90);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tripo|Teleport") bool bEntryEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tripo|Teleport") bool bRequireForwardDirection = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tripo|Teleport", meta=(ClampMin="-1",ClampMax="1")) float MinimumForwardDot = .7f;
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Tripo|Teleport") TObjectPtr<ATripoElevator> RequiredElevator;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tripo|Teleport", meta=(ClampMin="0",ClampMax="1")) int32 RequiredFloor = 1;
    UFUNCTION(BlueprintCallable, Category="Tripo|Teleport") bool TryTeleport(ATripoCharacter* Player);
private:
    double NextAllowedTime = 0.0;
};
