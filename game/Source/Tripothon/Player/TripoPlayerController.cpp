#include "Player/TripoPlayerController.h"
#include "InputKeyEventArgs.h"
#include "Lab/TripoHUD.h"

void ATripoPlayerController::KeyForTest(FKey Key, bool bPressed)
{
#if !UE_BUILD_SHIPPING
    InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1.f : 0.f));
#endif
}

bool ATripoPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
    // Menu shortcuts must also work while gameplay input is paused.
    if (Params.Key == EKeys::F2)
    {
        if (Params.Event == IE_Pressed)
            if (auto* HUD = Cast<ATripoHUD>(GetHUD())) HUD->HandleAction(TEXT("abilities.toggle"));
        return true;
    }
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
