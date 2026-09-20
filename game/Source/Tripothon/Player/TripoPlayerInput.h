#pragma once
#include "CoreMinimal.h"
#include "EnhancedPlayerInput.h"
#include "InputKeyEventArgs.h"
#include "TripoPlayerInput.generated.h"

// Preserve a digital tap until Enhanced Input has sampled its pressed state.
UCLASS()
class TRIPOTHON_API UTripoPlayerInput : public UEnhancedPlayerInput
{
    GENERATED_BODY()
public:
    virtual bool InputKey(const FInputKeyEventArgs& Params) override;
    virtual void ProcessInputStack(const TArray<UInputComponent*>& InputComponentStack, float DeltaTime, bool bGamePaused) override;
    virtual void FlushPressedKeys() override;
private:
    TSet<FKey> UnsampledPresses;
    TArray<FInputKeyEventArgs> DeferredReleases;
};
