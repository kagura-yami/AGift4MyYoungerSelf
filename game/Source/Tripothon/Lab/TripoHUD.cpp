#include "Lab/TripoHUD.h"
#include "Player/TripoCharacter.h"
#include "Engine/Canvas.h"
#include "World/TripoInteractionTarget.h"
#include "GameFramework/PlayerController.h"
void ATripoHUD::DrawHUD()
{
    Super::DrawHUD();
    RefreshUI();
    if(Canvas && !IsGameplayBlocked())
    {
        auto* P=Player(); auto* Target=P ? P->GetFocusedTarget() : nullptr;
        const FLinearColor C=Target ? FLinearColor(.58f,.84f,1.f,.9f) : FLinearColor(1,1,1,.65);
        const float X=Canvas->ClipX*.5f,Y=Canvas->ClipY*.5f;
        DrawRect(FLinearColor(0,0,0,.45),X-3,Y-3,6,6); DrawRect(C,X-1.5f,Y-1.5f,3,3);
        if(Target)
        {
            DrawLine(X-10,Y-6,X-10,Y+6,C,1.5); DrawLine(X+10,Y-6,X+10,Y+6,C,1.5);
            // Keep the object clear: center a separate keycap + label below the reticle.
            const FString Label=Target->Prompt.ToString();
            const float Scale=FMath::Clamp(Canvas->ClipY/900.f,1.f,1.35f);
            float W=0,H=0; GetTextSize(Label,W,H,nullptr,Scale);
            const float Key=24*Scale, Gap=10*Scale;
            const float Left=X-(Key+Gap+W)*.5f, Top=Y+32*Scale;
            DrawRect(FLinearColor(.05f,.12f,.18f,.65f),Left,Top+2*Scale,Key,Key);
            DrawRect(FLinearColor(.65f,.83f,.93f,.85f),Left,Top,Key,Key-2*Scale);
            float KW=0,KH=0; GetTextSize(TEXT("E"),KW,KH,nullptr,Scale);
            DrawText(TEXT("E"),FLinearColor(.07f,.15f,.2f,1),Left+(Key-KW)*.5f,Top+(Key-KH)*.5f,nullptr,Scale);
            const float TY=Top+(Key-H)*.5f;
            DrawText(Label,FLinearColor(0,0,0,.8f),Left+Key+Gap+1,TY+1,nullptr,Scale);
            DrawText(Label,FLinearColor(.8f,.92f,1,1),Left+Key+Gap,TY,nullptr,Scale);
        }
    }
}
