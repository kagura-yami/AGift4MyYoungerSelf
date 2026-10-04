#include "Lab/TripoHUD.h"
#include "Lab/TripoMenuStyle.h"
#include "Story/TripoStorySubsystem.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Engine/Texture2D.h"

TSharedRef<SWidget> ATripoHUD::BuildStoryBubble()
{
    const auto* Story=GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    FString Line=Story->GetLine(),Speaker=TEXT("旁白");
    int32 Split=INDEX_NONE;
    if ((Line.FindChar(TEXT('：'),Split) || Line.FindChar(TEXT(':'),Split)) && Split>0 && Split<16)
    { Speaker=Line.Left(Split); Line=Line.Mid(Split+1).TrimStartAndEnd(); }
    const bool bHero=Speaker==TEXT("主角");
    if (!StoryBubbleBrush.GetResourceObject())
    {
        // A feathered rounded silhouette with a linear horizontal alpha ramp.
        // Keep text separate so only the backdrop fades into the scene.
        constexpr int32 W=512,H=192;
        auto* Texture=UTexture2D::CreateTransient(W,H,PF_B8G8R8A8);
        Texture->SRGB=true; Texture->Filter=TF_Bilinear;
        auto& Mip=Texture->GetPlatformData()->Mips[0];
        auto* Pixels=static_cast<FColor*>(Mip.BulkData.Lock(LOCK_READ_WRITE));
        for(int32 Y=0;Y<H;++Y) for(int32 X=0;X<W;++X)
        {
            constexpr float Radius=27.f,Feather=9.f;
            const FVector2D Q(FMath::Abs(X+.5f-W*.5f)-(W*.5f-Radius-2.f),
                              FMath::Abs(Y+.5f-H*.5f)-(H*.5f-Radius-2.f));
            const float Distance=FVector2D(FMath::Max(Q.X,0.),FMath::Max(Q.Y,0.)).Size()+FMath::Min(FMath::Max(Q.X,Q.Y),0.)-Radius;
            const float Edge=FMath::Clamp(-Distance/Feather,0.f,1.f);
            const float Alpha=FMath::Lerp(.80f,.06f,float(X)/(W-1))*Edge;
            Pixels[Y*W+X]=FColor(28,42,38,FMath::RoundToInt(255*Alpha));
        }
        Mip.BulkData.Unlock(); Texture->UpdateResource();
        UITextures.Add(Texture);
        StoryBubbleBrush.SetResourceObject(Texture);
        StoryBubbleBrush.ImageSize=FVector2D(W,H);
        StoryBubbleBrush.DrawAs=ESlateBrushDrawType::Image;
    }
    const FLinearColor Ink(.94f,.93f,.86f);
    const FLinearColor Accent=bHero?FLinearColor(.69f,.83f,.78f):FLinearColor(.86f,.74f,.48f);
    auto Body=SNew(SVerticalBox);
    Body->AddSlot().AutoHeight().Padding(0,0,0,5)[TripoMenu::Label(Speaker,12,Accent)];
    auto DialogueText=TripoMenu::Label(Line,17,Ink);
    DialogueText->SetShadowOffset(FVector2D(0,1));
    DialogueText->SetShadowColorAndOpacity(FLinearColor(0,0,0,.6f));
    Body->AddSlot().AutoHeight()[DialogueText];
    if(Story->IsLastLine() && Story->GetCurrentId()==TEXT("Story.Finale.SendReply") && UTripoProgressSubsystem::Get(this)->GetReplyChoice()==INDEX_NONE)
        Body->AddSlot().AutoHeight().HAlign(HAlign_Right).Padding(0,8,0,0)
            [TripoMenu::Label(TEXT("1 收到啦   2 下次一起玩   3 再试一次"),10,FLinearColor(.66f,.69f,.66f))];
    return SNew(SBox).WidthOverride(400)
        [SNew(SBorder).BorderImage(&StoryBubbleBrush).BorderBackgroundColor(FLinearColor::White)
            .Padding(FMargin(20,14))[Body]];
}
