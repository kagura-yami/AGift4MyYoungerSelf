#include "Player/TripoPlayerController.h"
#include "InputKeyEventArgs.h"

void ATripoPlayerController::KeyForTest(FKey Key, bool bPressed)
{
#if !UE_BUILD_SHIPPING
    InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1.f : 0.f));
#endif
}

bool ATripoPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
    const bool bHandled = Super::InputKey(Params);
#if !UE_BUILD_SHIPPING
    if (Params.Key == EKeys::W || Params.Key == EKeys::SpaceBar || Params.Key == EKeys::Escape)
    {
        UE_LOG(LogTemp, Display, TEXT("TRIPO_INPUT key=%s event=%d handled=%d paused=%d"),
            *Params.Key.ToString(), int32(Params.Event), bHandled, IsPaused());
    }
#endif
    return bHandled;
}
