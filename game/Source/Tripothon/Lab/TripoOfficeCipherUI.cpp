#include "Lab/TripoHUD.h"
#include "Lab/TripoMenuStyle.h"
#include "World/TripoOfficeCipher.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Materials/MaterialInterface.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SCanvas.h"
#include "Widgets/SOverlay.h"

// Use the stationary surface's local coordinates so UI scaling and card motion
// cannot change the drag origin. Mouse capture also keeps a fast drag continuous.
class SCipherDragSurface : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCipherDragSurface) {}
        SLATE_ARGUMENT(TWeakObjectPtr<ATripoOfficeCipher>, Puzzle)
        SLATE_DEFAULT_SLOT(FArguments, Content)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args)
    {
        Puzzle=Args._Puzzle;
        SetClipping(EWidgetClipping::ClipToBounds);
        ChildSlot[Args._Content.Widget];
    }
    virtual FReply OnMouseButtonDown(const FGeometry& Geometry,const FPointerEvent& Event) override
    {
        if(Event.GetEffectingButton()==EKeys::LeftMouseButton && Puzzle.IsValid() && Puzzle->bCardOnPage)
        {
            const FVector2D Local=Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition());
            const FVector2D Relative=Local-FVector2D(80,32)-Puzzle->CardOffset;
            if(Relative.X>=0 && Relative.Y>=0 && Relative.X<=324 && Relative.Y<=576)
            {
                DragStart=Local; OffsetStart=Puzzle->CardOffset;
                return FReply::Handled().CaptureMouse(SharedThis(this));
            }
        }
        return FReply::Unhandled();
    }
    virtual FReply OnMouseMove(const FGeometry& Geometry,const FPointerEvent& Event) override
    {
        if(HasMouseCapture() && Puzzle.IsValid())
        {
            Puzzle->MoveCard(OffsetStart+Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition())-DragStart);
            return FReply::Handled();
        }
        return FReply::Unhandled();
    }
    virtual FReply OnMouseButtonUp(const FGeometry&,const FPointerEvent& Event) override
    {
        return Event.GetEffectingButton()==EKeys::LeftMouseButton && HasMouseCapture()
            ?FReply::Handled().ReleaseMouseCapture():FReply::Unhandled();
    }
private:
    TWeakObjectPtr<ATripoOfficeCipher> Puzzle;
    FVector2D DragStart,OffsetStart;
};

void ATripoHUD::ShowOfficeCipher(ATripoOfficeCipher* Puzzle,bool bLock)
{
    if(!IsValid(Puzzle)) return;
    OfficeCipher=Puzzle; bCipherLock=bLock; CipherCode.Empty();
    CipherCardBrush.SetResourceObject(Puzzle->CardUI); CipherCardBrush.ImageSize=FVector2D(324,576);
    CipherPageBrush=PaperBrush; CipherPageBrush.SetUVRegion(FBox2f(FVector2f(.22f,.22f),FVector2f(.76f,.76f)));
    if(auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) R->SetPauseReason(ETripoPauseReason::Reward,true);
    PanelKey.Empty();
}
void ATripoHUD::CloseOfficeCipher()
{
    if(!OfficeCipher.IsValid()) return;
    OfficeCipher.Reset();
    if(auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) R->SetPauseReason(ETripoPauseReason::Reward,false);
    PanelKey.Empty();
}
TSharedRef<SWidget> ATripoHUD::BuildOfficeCipher()
{
    const TWeakObjectPtr<ATripoHUD> H(this);
    const TWeakObjectPtr<ATripoOfficeCipher> P=OfficeCipher;
    auto Button=[this](const FString& Text,TFunction<void()> Action)
    {
        return SNew(SButton).ButtonStyle(&MenuButtonStyle()).ContentPadding(FMargin(18,11))
            .ButtonColorAndOpacity(FLinearColor(.09,.12,.12)).HAlign(HAlign_Center)
            .OnClicked_Lambda([Action]{Action(); return FReply::Handled();})
            [TripoMenu::Label(Text,16,TripoMenu::Paper,false,false)];
    };
    auto Column=SNew(SVerticalBox);
    auto Header=SNew(SHorizontalBox);
    Header->AddSlot().FillWidth(1)[TripoMenu::Label(bCipherLock?TEXT("保险箱密码"):TEXT("交接笔记"),28,TripoMenu::Paper,true)];
    Header->AddSlot().AutoWidth()[Button(TEXT("收起  Esc"),[H]{if(H.IsValid()) H->CloseOfficeCipher();})];
    Column->AddSlot().AutoHeight().Padding(0,0,0,20)[Header];
    if(bCipherLock)
    {
        Column->AddSlot().AutoHeight().Padding(0,0,0,16)[TripoMenu::Label(TEXT("四位数字。线索应该就在附近。"),16,TripoMenu::Paper)];
        auto Input=SNew(SEditableTextBox).Font(TripoMenu::Font(36,true)).Justification(ETextJustify::Center)
            .HintText(FText::FromString(TEXT("— — — —")))
            .OnTextChanged_Lambda([H,P](const FText& T){if(H.IsValid()){H->CipherCode=T.ToString();} if(P.IsValid())P->Feedback.Empty();})
            .OnVerifyTextChanged_Lambda([](const FText& T,FText& Error)
            {const FString S=T.ToString(); if(S.Len()>4){Error=FText::FromString(TEXT("密码为四位数字"));return false;} for(TCHAR C:S) if(C<TEXT('0')||C>TEXT('9')){Error=FText::FromString(TEXT("请输入数字"));return false;} return true;});
        Column->AddSlot().AutoHeight().Padding(0,0,0,16)[Input];
        Column->AddSlot().AutoHeight()[Button(TEXT("确认密码 · 前往目的地"),[H,P]
        {if(H.IsValid() && P.IsValid() && P->SubmitCode(H->Player(),H->CipherCode)) H->CloseOfficeCipher();})];
        Column->AddSlot().AutoHeight().Padding(0,16)[SNew(STextBlock).Font(TripoMenu::Font(16)).ColorAndOpacity(FLinearColor(.95,.65,.35))
            .Text_Lambda([P]{return P.IsValid()?FText::FromString(P->Feedback):FText::GetEmpty();})];
    }
    else
    {
        auto Row=SNew(SHorizontalBox);
        auto Tabs=SNew(SVerticalBox);
        for(int32 N=1;N<=7;++N)
        {
            Tabs->AddSlot().AutoHeight().Padding(0,0,12,8)
            [SNew(SButton).ButtonStyle(&MenuButtonStyle()).ContentPadding(FMargin(17,12))
                .ButtonColorAndOpacity_Lambda([P,N]{return P.IsValid() && P->Page==N?FLinearColor(.42,.29,.12):FLinearColor(.09,.12,.12);})
                .OnClicked_Lambda([P,N]{if(P.IsValid())P->SelectPage(N);return FReply::Handled();})
                [TripoMenu::Label(FString::Printf(TEXT("第 %d 页"),N),16,TripoMenu::Paper,false,false)]];
        }
        Row->AddSlot().AutoWidth().VAlign(VAlign_Center)[Tabs];
        auto PageCanvas=SNew(SCanvas);
        PageCanvas->AddSlot().Position(FVector2D(0,0)).Size(FVector2D(324,576))[SNew(SImage).Image(&CipherPageBrush)];
        PageCanvas->AddSlot().Position(FVector2D(24,24)).Size(FVector2D(276,80))
            [TripoMenu::Label(TEXT("工作备忘\n数字总在原来的位置。"),16,TripoMenu::Muted)];
        const FVector2D Holes[]={FVector2D(67.8,208.2),FVector2D(79,399.6),FVector2D(218.8,207.2),FVector2D(232.6,396)};
        for(int32 I=0;I<4;++I)
            PageCanvas->AddSlot().Position(Holes[I]-FVector2D(20,24)).Size(FVector2D(40,48))
            [SNew(STextBlock).Font(TripoMenu::Font(29,true)).Justification(ETextJustify::Center).ColorAndOpacity(TripoMenu::Ink)
                .Text_Lambda([P,I]{return FText::AsNumber(P.IsValid()?P->Digit(P->Page,I):0);})];
        // Extra print keeps the unmasked page a field of numbers, not a pre-solved code.
        for(int32 I=0;I<12;++I)
            PageCanvas->AddSlot().Position(FVector2D(35+(I%4)*70,115+(I/4)*175)).Size(FVector2D(38,40))
            [SNew(STextBlock).Font(TripoMenu::Font(18)).ColorAndOpacity(TripoMenu::Muted)
                .Text_Lambda([P,I]{return FText::AsNumber(P.IsValid()?(P->Page*3+I*7)%10:0);})];
        PageCanvas->AddSlot().Position_Lambda([P]{return P.IsValid()?P->CardOffset:FVector2D::ZeroVector;}).Size(FVector2D(324,576))
            [SNew(SImage).Image(&CipherCardBrush).Visibility_Lambda([P]{return P.IsValid() && P->bCardOnPage?EVisibility::HitTestInvisible:EVisibility::Collapsed;})];
        Row->AddSlot().AutoWidth()[SNew(SBox).WidthOverride(484).HeightOverride(640)
            [SNew(SCipherDragSurface).Puzzle(P)
                [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("NoBrush")).Padding(FMargin(80,32))[PageCanvas]]]];
        auto Notes=SNew(SVerticalBox);
        Notes->AddSlot().AutoHeight()[TripoMenu::Label(TEXT("留给接手的人"),22,TripoMenu::Paper,true)];
        Notes->AddSlot().AutoHeight().Padding(0,16,0,28)[TripoMenu::Label(TEXT("卡片旁的数字是页码。\n沿着箭头，找出每个孔里藏着的数字。"),17,TripoMenu::Paper)];
        Notes->AddSlot().AutoHeight()[SNew(SButton).ButtonStyle(&MenuButtonStyle()).ContentPadding(FMargin(18,14))
            .ButtonColorAndOpacity(FLinearColor(.09,.12,.12))
            .IsEnabled_Lambda([P]{return P.IsValid() && P->bHasCard;})
            .OnClicked_Lambda([P]{if(P.IsValid())P->ToggleCard();return FReply::Handled();})
            [SNew(STextBlock).Font(TripoMenu::Font(18)).ColorAndOpacity(TripoMenu::Paper)
                .Text_Lambda([P]{return FText::FromString(!P.IsValid() || !P->bHasCard?TEXT("还没有打孔卡"):P->bCardOnPage?TEXT("取下卡片"):TEXT("放上打孔卡"));})]];
        Notes->AddSlot().AutoHeight().Padding(0,18)[TripoMenu::Label(TEXT("按住鼠标左键拖动打孔卡。\n翻页会保留卡片的位置。\n记下数字，在保险箱输入密码。"),14,FLinearColor(.64,.66,.59))];
        Row->AddSlot().AutoWidth().Padding(30,24,0,0)[SNew(SBox).WidthOverride(240)[Notes]];
        Column->AddSlot().AutoHeight()[Row];
    }
    return SNew(SBox).WidthOverride(bCipherLock?510:1020)
        [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.024,.028,.026,.98)).Padding(30)[Column]];
}
