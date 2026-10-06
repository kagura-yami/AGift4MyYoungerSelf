#pragma once
#include "CoreMinimal.h"
#include "World/TripoZone.h"
#include "TripoGiftFinish.generated.h"
class ATripoGiftBox;
class UStaticMeshComponent;
class UPointLightComponent;
// Single-player finish: placed boxes own receipts, this actor owns presentation only.
UCLASS()
class TRIPOTHON_API ATripoGiftFinish : public ATripoZone
{
    GENERATED_BODY()
public:
    ATripoGiftFinish();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditInstanceOnly,BlueprintReadWrite,Category="Reward") TArray<TObjectPtr<ATripoGiftBox>> GiftBoxes;
private:
    void Reveal(int32 Count,bool bBurst);
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Sparks;
    UPROPERTY() TArray<TObjectPtr<UPointLightComponent>> Lights;
    double BurstAt=-1;
    int32 ShownCount=0;
};
