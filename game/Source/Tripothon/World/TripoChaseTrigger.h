#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoChaseTrigger.generated.h"
class UBoxComponent;
class UArrowComponent;
class ATripoChaser;
class ATripoCharacter;
class ATripoChaseHideZone;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTripoChaseStarted, ATripoChaser*, Chaser);

/** Owns the spawned pursuer. Recovery clears it; walking into the volume again starts a new attempt. */
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoChaseTrigger : public AActor
{
    GENERATED_BODY()
public:
    ATripoChaseTrigger();
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chase") TObjectPtr<UBoxComponent> Volume;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chase") TObjectPtr<UArrowComponent> SpawnMarker;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase") TSubclassOf<ATripoChaser> ChaserClass;
    // Optional level TargetPoint. Otherwise use the movable SpawnMarker component.
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Chase") TObjectPtr<AActor> SpawnPoint;
    // Entering one of these safe rooms ends only this trigger's encounter.
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Chase") TArray<TObjectPtr<ATripoChaseHideZone>> ExitZones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase") bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase") bool bRearmAfterRestore = true;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Chase") TObjectPtr<ATripoChaser> ActiveChaser;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Chase") FString LastError;
    UPROPERTY(BlueprintAssignable, Category="Chase") FTripoChaseStarted OnChaseStarted;
    UFUNCTION(BlueprintCallable, Category="Chase") bool ActivateChase(ATripoCharacter* Player);
    // Editor convenience for constructing/testing the whitebox after moving its geometry.
    UFUNCTION(BlueprintCallable, CallInEditor, Category="Chase|Setup") void BuildNavigationForLevel();
    // Call from an exit/finish Blueprint. Does not kill or reset the player.
    UFUNCTION(BlueprintCallable, Category="Chase") void EndChase(bool bAllowRetrigger = false);
private:
    bool bWasInside = false;
    bool bUsed = false;
    int64 ActivationEpoch = 0;
};
