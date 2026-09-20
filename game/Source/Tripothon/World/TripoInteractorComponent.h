#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TripoInteractorComponent.generated.h"

UENUM(BlueprintType)
enum class ETripoInteractor : uint8 { Player, Echo, Memory };

UCLASS(ClassGroup=(Tripo), meta=(BlueprintSpawnableComponent))
class TRIPOTHON_API UTripoInteractorComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTripoInteractorComponent();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) ETripoInteractor Kind = ETripoInteractor::Memory;
    UPROPERTY(BlueprintReadOnly) bool bSuppressed = false;
    bool CanTrigger(bool bAllowEcho) const;
};
