#include "Lab/TripoGameMode.h"
#include "Player/TripoCharacter.h"
#include "Lab/TripoHUD.h"
#include "Player/TripoPlayerController.h"
ATripoGameMode::ATripoGameMode()
{
    DefaultPawnClass = ATripoCharacter::StaticClass();
    HUDClass = ATripoHUD::StaticClass();
    PlayerControllerClass = ATripoPlayerController::StaticClass();
}

ATripoFrontEndMode::ATripoFrontEndMode()
{
    DefaultPawnClass = nullptr;
    HUDClass = ATripoHUD::StaticClass();
    PlayerControllerClass = ATripoPlayerController::StaticClass();
}
