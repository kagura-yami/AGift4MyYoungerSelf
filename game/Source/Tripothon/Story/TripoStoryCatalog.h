#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TripoStoryCatalog.generated.h"
USTRUCT(BlueprintType)
struct FTripoStoryEvent
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Id;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Title;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true)) FText Text;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> GrantFloors;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> RequiredEvents;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> RequiredChallenges;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName NextMap;
};
UCLASS(BlueprintType)
class TRIPOTHON_API UTripoStoryCatalog : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FTripoStoryEvent> Events;
    const FTripoStoryEvent* Find(FName Id) const;
    bool IsValidCatalog() const;
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
