#pragma once
#include "Abilities/TripoAbilityInstance.h"
#include "TripoAbilityProbe.generated.h"

// Explicit automation fixture, never referenced by the default ability catalog.
UCLASS(Transient, NotBlueprintable)
class UTripoAbilityProbe : public UTripoAbilityInstance
{
    GENERATED_BODY()
public:
    virtual ETripoAbilityFailure Validate(AActor*, const FTripoAbilityParameters&) const override;
    virtual ETripoAbilityFailure BeginEffect(AActor*, const FTripoAbilityParameters&) override;
    virtual void UpdateEffect(double Delta) override { UpdatedSeconds += Delta; }
    virtual void EndEffect(bool bCancelled) override { ++Ends; bWasCancelled = bCancelled; }
    double UpdatedSeconds = 0;
    int32 Ends = 0;
    bool bWasCancelled = false;
};
