#pragma once
#include "Abilities/TripoAbilityInstance.h"
#include "TripoMovementAbilities.generated.h"
class ATripoCharacter;
UCLASS()
class UTripoDashAbility : public UTripoAbilityInstance
{
    GENERATED_BODY()
public:
    virtual ETripoAbilityFailure Validate(AActor*, const FTripoAbilityParameters&) const override;
    virtual ETripoAbilityFailure BeginEffect(AActor*, const FTripoAbilityParameters&) override;
    virtual void UpdateEffect(double Delta) override;
    virtual void EndEffect(bool bCancelled) override;
protected:
    virtual bool IsUp() const { return false; }
    ATripoCharacter* Player() const;
};
UCLASS()
class UTripoUpDashAbility : public UTripoDashAbility
{
    GENERATED_BODY()
protected:
    virtual bool IsUp() const override { return true; }
};
UCLASS()
class UTripoWallJumpAbility : public UTripoAbilityInstance
{
    GENERATED_BODY()
public:
    virtual ETripoAbilityFailure Validate(AActor*, const FTripoAbilityParameters&) const override;
    virtual ETripoAbilityFailure BeginEffect(AActor*, const FTripoAbilityParameters&) override;
};
