#pragma once
#include "CoreMinimal.h"
#include "TripoGiftReceipt.generated.h"

USTRUCT(BlueprintType)
struct FTripoGiftReceipt
{
    GENERATED_BODY()
    // -1 means a commemorative gift: no eligible upgrades remained.
    UPROPERTY(BlueprintReadOnly) int32 AbilityIndex = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) int32 GrantedLevel = 0;
    bool IsValid() const
    {
        return AbilityIndex == INDEX_NONE ? GrantedLevel == 0 : AbilityIndex >= 0 && AbilityIndex < 8 && GrantedLevel >= 1 && GrantedLevel <= 3;
    }
};
