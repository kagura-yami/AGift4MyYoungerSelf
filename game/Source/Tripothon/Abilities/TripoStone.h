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
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
private:
    double EndsAt = 0;
};
