#pragma once
#include "Abilities/TripoAbilityInstance.h"
#include "TripoWorldAbilities.generated.h"
class ATripoCharacter;
class ATripoMechanism;
UCLASS()
class UTripoStoneAbility : public UTripoAbilityInstance
{
    GENERATED_BODY()
public:
    virtual ETripoAbilityFailure Validate(AActor*, const FTripoAbilityParameters&) const override;
    virtual ETripoAbilityFailure BeginEffect(AActor*, const FTripoAbilityParameters&) override;
    static ETripoAbilityFailure CheckPlacement(ATripoCharacter* Player, const FTripoAbilityParameters& Parameters, FVector& Location);
    virtual double GetExecutionDuration(const FTripoAbilityParameters&) const override { return 0; }
};
UCLASS()
class UTripoSlowAbility : public UTripoAbilityInstance
{
    GENERATED_BODY()
public:
    virtual ETripoAbilityFailure Validate(AActor*, const FTripoAbilityParameters&) const override;
    virtual ETripoAbilityFailure BeginEffect(AActor*, const FTripoAbilityParameters&) override;
    virtual void UpdateEffect(double Delta) override;
    virtual void EndEffect(bool bCancelled) override;
private:
    TWeakObjectPtr<ATripoMechanism> Mechanism;
    FGuid SlowHandle;
};
