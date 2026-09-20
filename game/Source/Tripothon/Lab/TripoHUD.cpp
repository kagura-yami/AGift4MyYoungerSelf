#include "Lab/TripoHUD.h"
#include "Player/TripoCharacter.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
void ATripoHUD::DrawHUD()
{
    Super::DrawHUD();
    RefreshUI();
}
