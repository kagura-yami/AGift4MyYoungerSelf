#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TripoIdentityRegistry.generated.h"

class UTripoIdentityComponent;

UCLASS()
class TRIPOTHON_API UTripoIdentityRegistry : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, Category="Tripo|Identity", meta=(WorldContext="WorldContextObject"))
    static UTripoIdentityRegistry* GetRegistry(const UObject* WorldContextObject);
    bool Register(UTripoIdentityComponent* Identity);
    void Unregister(UTripoIdentityComponent* Identity);
    UFUNCTION(BlueprintPure, Category="Tripo|Identity") AActor* Resolve(FGuid Id) const;
private:
    TMap<FGuid, TWeakObjectPtr<UTripoIdentityComponent>> Entries;
    // Ambiguity stays quarantined until the world is reloaded with repaired data.
    TSet<FGuid> Conflicts;
};
