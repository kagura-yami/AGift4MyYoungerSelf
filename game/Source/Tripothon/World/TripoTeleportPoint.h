#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoTeleportPoint.generated.h"
class UArrowComponent;
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
    bool TryTeleport(ATripoCharacter* Player);
private:
    double NextAllowedTime = 0.0;
};
