#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoChaseHideZone.generated.h"
class UBoxComponent;
class UTextRenderComponent;
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoChaseHideZone : public AActor
{
    GENERATED_BODY()
public:
    ATripoChaseHideZone();
    virtual void OnConstruction(const FTransform& Transform) override;
    UFUNCTION(BlueprintCallable, Category="Chase") void SetEnabled(bool bNewEnabled);
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chase") TObjectPtr<UBoxComponent> Volume;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Chase") TObjectPtr<UTextRenderComponent> Label;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase") bool bEnabled = true;
    // Zero means instant clearing. Overlapping zones use the strongest reduction.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chase", meta=(ClampMin="0")) float AggroReductionPerSecond = 45.f;
    UFUNCTION(BlueprintPure, Category="Chase") bool ContainsPoint(FVector Point) const;
};
