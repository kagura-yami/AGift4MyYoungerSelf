#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "TripoIdentityComponent.generated.h"

UCLASS(ClassGroup=(Tripo), meta=(BlueprintSpawnableComponent))
class TRIPOTHON_API UTripoIdentityComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTripoIdentityComponent();
    virtual void OnRegister() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
#if WITH_EDITOR
    virtual void PostEditImport() override;
#endif
    UFUNCTION(BlueprintPure, Category="Tripo|Identity") FGuid GetStableId() const { return StableId; }
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tripo|Identity") FGameplayTag SourceType;
private:
    UPROPERTY(VisibleInstanceOnly, Category="Tripo|Identity") FGuid StableId;
};
