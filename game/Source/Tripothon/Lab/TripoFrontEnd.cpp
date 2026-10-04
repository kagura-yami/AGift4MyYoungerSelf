#include "Lab/TripoHUD.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Lab/TripoMenuStyle.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Brushes/SlateColorBrush.h"

TSharedRef<SWidget> ATripoHUD::BuildFrontEnd()
{
    const TWeakObjectPtr<ATripoHUD> Weak(this);
    const bool HasSave=UTripoProgressSubsystem::Get(this)->HasCompatibleSave(false);
    const FLinearColor Gold(.9f,.68f,.34f),Cream(.98f,.94f,.82f);
    // Slate owns the animation state; weak widget/HUD references avoid ownership cycles.
    struct FButtonMotion { float Hover=0, Press=0, Flash=0; double Trigger=-1; };
    const auto Pending=MakeShared<bool>(false);
    static const FButtonStyle Style=FButtonStyle()
        .SetNormal(FSlateColorBrush(FLinearColor::Transparent))
        .SetHovered(FSlateColorBrush(FLinearColor::Transparent))
        .SetPressed(FSlateColorBrush(FLinearColor::Transparent))
        .SetNormalPadding(FMargin(0)).SetPressedPadding(FMargin(0));
    static const FSlateColorBrush White(FLinearColor::White);
    auto Button=[Weak,Gold,Cream,Pending](const FString& Text,const FString& Action,bool Primary=false,bool Enabled=true)
    {
        const auto Motion=MakeShared<FButtonMotion>();
        auto B=SNew(SButton).ButtonStyle(&Style).IsEnabled(Enabled).ContentPadding(FMargin(0))
            .OnClicked_Lambda([Motion,Pending]{
                if(!*Pending) { *Pending=true; Motion->Trigger=FPlatformTime::Seconds(); }
                return FReply::Handled();
            });
        TWeakPtr<SButton> Hover=B;
        B->SetRenderTransformPivot(FVector2D(0,.5f));
        B->RegisterActiveTimer(0,FWidgetActiveTimerDelegate::CreateLambda(
            [Hover,Motion,Weak,Action,Pending](double,float Delta){
                auto P=Hover.Pin();
                if(!P.IsValid()) return EActiveTimerReturnType::Stop;
                const bool Hot=P->IsEnabled() && (P->IsHovered() || P->HasKeyboardFocus());
                Motion->Hover=FMath::FInterpTo(Motion->Hover,Hot?1.f:0.f,Delta,12.f);
                Motion->Press=FMath::FInterpTo(Motion->Press,P->IsPressed()?1.f:0.f,Delta,24.f);
                if(Motion->Trigger>=0) {
                    const float Age=float(FPlatformTime::Seconds()-Motion->Trigger);
                    Motion->Flash=FMath::Sin(FMath::Clamp(Age/.16f,0.f,1.f)*PI);
                    if(Age>=.16f) {
                        Motion->Trigger=-1; Motion->Flash=0; *Pending=false;
                        if(Weak.IsValid()) Weak->HandleAction(Action);
                        return EActiveTimerReturnType::Stop;
                    }
                }
                P->SetRenderTransform(FSlateRenderTransform(FScale2D(1.f-.018f*Motion->Press),
                    FVector2D(6.f*Motion->Hover,2.f*Motion->Press)));
                P->Invalidate(EInvalidateWidgetReason::Paint);
                return EActiveTimerReturnType::Continue;
            }));
        B->SetContent(SNew(SOverlay)
            +SOverlay::Slot()
                [SNew(SBorder).BorderImage(&White).Padding(0).Visibility(EVisibility::HitTestInvisible)
                    .BorderBackgroundColor_Lambda([Motion]{return FLinearColor(.65f,.46f,.2f,.085f*Motion->Hover+.16f*Motion->Flash);})]
            +SOverlay::Slot().VAlign(VAlign_Bottom).HAlign(HAlign_Left).Padding(38,0,0,2)
                [SNew(SBox).HeightOverride(1)
                    .WidthOverride_Lambda([Motion]{return 12.f+220.f*Motion->Hover;})
                    [SNew(SBorder).Padding(0).BorderImage(&White).Visibility(EVisibility::HitTestInvisible)
                        .BorderBackgroundColor_Lambda([Motion,Gold]{return Gold.CopyWithNewOpacity(.55f*Motion->Hover+.4f*Motion->Flash);})]]
            +SOverlay::Slot().Padding(8,9)
                [SNew(SHorizontalBox)
                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,16,0)
                    [SNew(STextBlock).Text(FText::FromString(TEXT("◇"))).Font(TripoMenu::Font(14))
                        .ShadowOffset(FVector2D(0,1)).ShadowColorAndOpacity(FLinearColor(0,0,0,.8f))
                        .ColorAndOpacity_Lambda([Motion,Gold,Primary]{return FSlateColor(FMath::Lerp(
                            Primary?Gold:FLinearColor(.6f,.57f,.43f,.5f),FLinearColor(1,.88f,.55f),Motion->Hover));})]
                +SHorizontalBox::Slot().AutoWidth()
                    [SNew(STextBlock).Text(FText::FromString(Text)).Font(TripoMenu::Font(22,true))
                        .ShadowOffset(FVector2D(1,2)).ShadowColorAndOpacity(FLinearColor(0,0,0,.65f))
                        .ColorAndOpacity_Lambda([Motion,Gold,Cream,Primary]{return FSlateColor(FMath::Lerp(
                            Primary?Gold:Cream,FLinearColor(1,.85f,.5f),FMath::Max(Motion->Hover,Motion->Flash)));})]]);
        return B;
    };
    auto Nav=SNew(SVerticalBox);
    Nav->AddSlot().AutoHeight()[TripoMenu::Label(TEXT("A GIFT FOR MY YOUNGER SELF"),11,Gold,false,false)];
    Nav->AddSlot().AutoHeight().Padding(0,18,0,0)[TripoMenu::Label(TEXT("明天寄来的"),38,Cream,true,false)];
    Nav->AddSlot().AutoHeight().Padding(0,-6,0,0)[TripoMenu::Label(TEXT("礼物"),82,Cream,true,false)];
    Nav->AddSlot().AutoHeight().Padding(0,12,0,32)[TripoMenu::Label(TEXT("把未说完的话，寄给从前的自己。"),14,FLinearColor(.76f,.77f,.64f),false,false)];
    if(bConfirmNewGame)
    {
        Nav->AddSlot().AutoHeight().Padding(0,0,0,12)[TripoMenu::Label(TEXT("开始新的旅程？\n旧存档会保留到下一次保存。"),16,Cream)];
        Nav->AddSlot().AutoHeight()[Button(TEXT("出发"),TEXT("front.start"),true)];
        Nav->AddSlot().AutoHeight()[Button(TEXT("再想一想"),TEXT("front.cancel"))];
    }
    else
    {
        Nav->AddSlot().AutoHeight()[Button(HasSave?TEXT("继续旅程"):TEXT("暂无安全存档"),TEXT("load"),HasSave,HasSave)];
        Nav->AddSlot().AutoHeight()[Button(TEXT("新的旅程"),TEXT("new"),!HasSave)];
        Nav->AddSlot().AutoHeight()[Button(TEXT("设置与教程"),TEXT("ui.settings"))];
        Nav->AddSlot().AutoHeight()[Button(TEXT("退出游戏"),TEXT("quit"))];
    }
    Nav->AddSlot().AutoHeight().Padding(0,12)[TripoMenu::Label(FrontEndMessage,13,Gold)];
    const double Start=FPlatformTime::Seconds();
    return SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("NoBrush")).Padding(0)
        .ColorAndOpacity_Lambda([Start]{return FLinearColor(1,1,1,float(FMath::Clamp((FPlatformTime::Seconds()-Start)/.45,0.,1.)));})
        [SNew(SBox).WidthOverride(1280).HeightOverride(720)
        [SNew(SOverlay)
            +SOverlay::Slot().Padding(70,44,810,50)[Nav]
            +SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(78,0,0,16)
                [TripoMenu::Label(TEXT("TRIPOTHON  /  写给从前，留给明天"),11,FLinearColor(.65f,.68f,.57f),false,false)]]];
}
