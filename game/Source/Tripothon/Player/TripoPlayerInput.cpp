#include "Player/TripoPlayerInput.h"

bool UTripoPlayerInput::InputKey(const FInputKeyEventArgs& Params)
{
    if (Params.Event == IE_Pressed && !Params.Key.IsAnalog())
    {
        DeferredReleases.RemoveAll([&Params](const FInputKeyEventArgs& Release) { return Release.Key == Params.Key; });
        UnsampledPresses.Add(Params.Key);
    }
    else if (Params.Event == IE_Released && UnsampledPresses.Contains(Params.Key))
    {
        DeferredReleases.Add(Params);
        return true;
    }
    return Super::InputKey(Params);
}

void UTripoPlayerInput::ProcessInputStack(const TArray<UInputComponent*>& Stack, float DeltaTime, bool bGamePaused)
{
    Super::ProcessInputStack(Stack, DeltaTime, bGamePaused);
    UnsampledPresses.Reset();
    for (const FInputKeyEventArgs& Release : DeferredReleases) Super::InputKey(Release);
    DeferredReleases.Reset();
}

void UTripoPlayerInput::FlushPressedKeys()
{
    UnsampledPresses.Reset();
    DeferredReleases.Reset();
    Super::FlushPressedKeys();
}
