#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TripoPlayerSettings.generated.h"
UCLASS(Config=GameUserSettings)
class TRIPOTHON_API UTripoPlayerSettings : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(Config, BlueprintReadOnly) float MasterVolume = 1.f;
    UPROPERTY(Config, BlueprintReadOnly) float MouseSensitivity = 1.5f;
    UPROPERTY(Config, BlueprintReadOnly) bool bInvertLookY = false;
    UFUNCTION(BlueprintPure) static UTripoPlayerSettings* Get() { return GetMutableDefault<UTripoPlayerSettings>(); }
    UFUNCTION(BlueprintCallable) void SetVolume(float Value, UObject* Context);
    UFUNCTION(BlueprintCallable) void SetSensitivity(float Value);
    UFUNCTION(BlueprintCallable) void SetInvertY(bool Value);
    void ApplyAudio(UObject* Context);
};
