#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoStoryTrigger.generated.h"
class UBoxComponent;
class UPrimitiveComponent;
class UTextRenderComponent;
class UTripoStoryCatalog;
class ATripoCharacter;
UCLASS()
class TRIPOTHON_API ATripoStoryTrigger : public AActor
{
    GENERATED_BODY()
public:
    ATripoStoryTrigger();
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    bool Interact(ATripoCharacter* Player);
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Volume;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UTripoStoryCatalog> Catalog;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName EventId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Hint = TEXT("E / inspect");
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAutomatic = true;
private:
    UFUNCTION() void Enter(UPrimitiveComponent* Component, AActor* Other, UPrimitiveComponent* OtherComponent, int32 BodyIndex, bool bSweep, const FHitResult& Hit);
};
