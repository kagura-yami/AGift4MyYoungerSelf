#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TripoAbilityDefinition.generated.h"

UENUM(BlueprintType)
enum class ETripoAbility : uint8 { Dash, UpDash, WallJump, StepStone, Slow, Rewind, Echo, BonusTime };

UENUM(BlueprintType)
enum class ETripoAbilityFailure : uint8
{
    None, Locked, InvalidDefinition, NoTarget, Cooldown, AlreadyActive, Paused,
    Restoring, NotImplemented, Passive, InvalidContext, Blocked, Capacity, AirUseSpent
};

USTRUCT(BlueprintType)
struct TRIPOTHON_API FTripoAbilityParameters
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Cooldown = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Duration = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Distance = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float Strength = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Capacity = 1;
};

class UTripoAbilityInstance;
UCLASS(BlueprintType)
class TRIPOTHON_API UTripoAbilityDefinition : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) ETripoAbility Ability = ETripoAbility::Dash;
    // Exactly three entries: level zero is locked and has no parameters.
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FTripoAbilityParameters> Levels;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bRequiresTarget = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSubclassOf<UTripoAbilityInstance> Implementation;
    bool IsValidDefinition() const;
#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
    const FTripoAbilityParameters* Parameters(int32 Level) const;
    static UTripoAbilityDefinition* MakeDefaults(UObject* Outer, ETripoAbility Ability);
};
