#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Abilities/TripoAbilityDefinition.h"
#include "TripoAbilityComponent.generated.h"

class UTripoAbilityInstance;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FTripoAbilityEvent, ETripoAbility, Ability, FGuid, Handle, ETripoAbilityFailure, Failure, bool, bCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTripoAbilityHitEvent, ETripoAbility, Ability, FGuid, Handle, const FHitResult&, Hit);

UCLASS(ClassGroup=(Tripo), meta=(BlueprintSpawnableComponent))
class TRIPOTHON_API UTripoAbilityComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTripoAbilityComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Delta, ELevelTick TickType, FActorComponentTickFunction* Function) override;
    UFUNCTION(BlueprintCallable, Category="Tripo|Abilities") bool InitializeDefinitions();
    UFUNCTION(BlueprintCallable, Category="Tripo|Abilities") bool GrantLevelFloor(ETripoAbility Ability, int32 MinimumLevel);
    UFUNCTION(BlueprintPure, Category="Tripo|Abilities") int32 GetLevel(ETripoAbility Ability) const;
    UFUNCTION(BlueprintPure, Category="Tripo|Abilities") bool MeetsLevelFloor(ETripoAbility Ability, int32 RequiredLevel) const;
    UFUNCTION(BlueprintCallable, Category="Tripo|Abilities") ETripoAbilityFailure TryActivate(ETripoAbility Ability, AActor* Target, FGuid& Handle);
    UFUNCTION(BlueprintCallable, Category="Tripo|Abilities") bool Cancel(FGuid Handle);
    UFUNCTION(BlueprintCallable) bool CancelAbility(ETripoAbility Id);
    UFUNCTION(BlueprintCallable, Category="Tripo|Abilities") void CancelAll();
    // Effects report confirmed hits only; invalid/stale handles produce no event.
    bool ReportHit(FGuid Handle, const FHitResult& Hit);
    UFUNCTION(BlueprintCallable, Category="Tripo|Abilities") double GetCooldownRemaining(ETripoAbility Ability) const;
    // Challenge start snapshots this value once; this getter never modifies any clock.
    UFUNCTION(BlueprintPure, Category="Tripo|Abilities") float GetBonusTimeSeconds() const;
    TArray<int32> ExportLevels() const;
    bool ImportLevels(const TArray<int32>& Values);
    TArray<double> ExportCooldowns() const;
    bool ImportCooldowns(const TArray<double>& Values);
    UFUNCTION(BlueprintPure) bool HasActiveAbility(ETripoAbility Id) const;
    UFUNCTION(BlueprintPure) bool GetParameters(ETripoAbility Id, FTripoAbilityParameters& Parameters) const;
    void ReportFailure(ETripoAbility Id, ETripoAbilityFailure Failure);
    FString GetFailureMessage() const;
    UPROPERTY(EditAnywhere, Category="Tripo|Abilities") TArray<TObjectPtr<UTripoAbilityDefinition>> Definitions;
    UPROPERTY(BlueprintAssignable) FTripoAbilityEvent OnStarted;
    UPROPERTY(BlueprintAssignable) FTripoAbilityEvent OnFailed;
    UPROPERTY(BlueprintAssignable) FTripoAbilityEvent OnEnded;
    UPROPERTY(BlueprintAssignable) FTripoAbilityHitEvent OnHit;
private:
    UPROPERTY() TMap<ETripoAbility, TObjectPtr<UTripoAbilityDefinition>> Catalog;
    UPROPERTY() TMap<ETripoAbility, TObjectPtr<UTripoAbilityInstance>> Instances;
    TMap<ETripoAbility, int32> Levels;
    TMap<ETripoAbility, double> ReadyAt;
    bool bInitialized = false;
    bool bMutating = false;
    bool bEndingPlay = false;
    ETripoAbilityFailure LastFailure = ETripoAbilityFailure::None;
    void Finish(ETripoAbility Ability, UTripoAbilityInstance* Instance, bool bCancelled);
};
