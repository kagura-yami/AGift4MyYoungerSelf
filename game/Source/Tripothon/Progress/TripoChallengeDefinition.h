#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Abilities/TripoAbilityDefinition.h"
#include "TripoChallengeDefinition.generated.h"
USTRUCT(BlueprintType)
struct FTripoRewardOption
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ETripoAbility Ability = ETripoAbility::Dash;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Weight = 1;
};
UCLASS(BlueprintType, EditInlineNew)
class TRIPOTHON_API UTripoChallengeDefinition : public UDataAsset
{
    GENERATED_BODY()
public:
    UTripoChallengeDefinition();
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ChallengeId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) double Budget = 30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<int32> RequiredLevels;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FTripoRewardOption> Rewards;
    bool IsValidDefinition() const;
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
