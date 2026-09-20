#pragma once
#include "CoreMinimal.h"
#include "Abilities/TripoAbilityDefinition.h"
#include "TripoAbilityInstance.generated.h"

// One owned UObject per ability per character. Component supplies ActionClock time.
// Validate must have no side effects; BeginEffect must commit atomically or undo its own failure.
UCLASS(Abstract)
class TRIPOTHON_API UTripoAbilityInstance : public UObject
{
    GENERATED_BODY()
public:
    virtual ETripoAbilityFailure Validate(AActor* Target, const FTripoAbilityParameters& Parameters) const;
    virtual ETripoAbilityFailure BeginEffect(AActor* Target, const FTripoAbilityParameters& Parameters);
    virtual void UpdateEffect(double ActionDelta) {}
    virtual void EndEffect(bool bCancelled) {}
    virtual double GetExecutionDuration(const FTripoAbilityParameters& Parameters) const { return Parameters.Duration; }
    bool IsActive() const { return Handle.IsValid(); }
    FGuid GetHandle() const { return Handle; }
    // Effects may request an early normal finish during UpdateEffect (e.g. blocking hit).
    void RequestCompletion() { if (IsActive()) bCompletionRequested = true; }
private:
    friend class UTripoAbilityComponent;
    FGuid Handle;
    double EndsAt = 0;
    double LastUpdate = 0;
    bool bCompletionRequested = false;
};
