#pragma once
#include "Abilities/TripoAbilityInstance.h"
#include "Time/TripoHistoryComponent.h"
#include "TripoRewindAbility.generated.h"
UCLASS()
class UTripoRewindAbility : public UTripoAbilityInstance
{
    GENERATED_BODY()
public:
    virtual ETripoAbilityFailure Validate(AActor*, const FTripoAbilityParameters&) const override;
    virtual ETripoAbilityFailure BeginEffect(AActor*, const FTripoAbilityParameters&) override;
    virtual void UpdateEffect(double Delta) override;
    virtual void EndEffect(bool bCancelled) override;
    virtual double GetExecutionDuration(const FTripoAbilityParameters&) const override { return Length; }
private:
    UPROPERTY() TArray<FTripoHistoryFrame> Frames;
    TWeakObjectPtr<AActor> Subject;
    TWeakObjectPtr<UTripoHistoryComponent> History;
    double Cursor = 0;
    double Length = 0;
    int64 Epoch = 0;
};
