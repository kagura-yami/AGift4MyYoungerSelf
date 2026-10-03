#pragma once
#include "Abilities/TripoAbilityInstance.h"
#include "TripoEchoAbility.generated.h"
class ATripoEchoActor;
UCLASS()
class UTripoEchoAbility : public UTripoAbilityInstance
{
    GENERATED_BODY()
public:
    virtual ETripoAbilityFailure Validate(AActor*, const FTripoAbilityParameters&) const override;
    virtual ETripoAbilityFailure BeginEffect(AActor*, const FTripoAbilityParameters&) override;
};
