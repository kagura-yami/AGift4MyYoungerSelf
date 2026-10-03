#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoStone.generated.h"
class UStaticMeshComponent;
class UTextRenderComponent;
UCLASS()
class TRIPOTHON_API ATripoStone : public AActor
{
    GENERATED_BODY()
public:
    ATripoStone();
    virtual void BeginPlay() override;
    virtual void Tick(float Delta) override;
    float Duration = 8;
    double GetRemainingLifetime() const;
    UFUNCTION(BlueprintPure, Category="Tripo|Stone") bool IsUsable() const { return !bShattering; }
    UFUNCTION(BlueprintCallable, Category="Tripo|Stone") void Shatter();
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
private:
    double EndsAt = 0;
    double BreakStarted = 0;
    bool bShattering = false;
    UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Fragments;
    TArray<FVector> FragmentOrigins;
    TArray<FVector> FragmentVelocities;
};
