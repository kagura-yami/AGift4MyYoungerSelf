#include "Lab/TripoHUD.h"
#include "Lab/TripoMenuStyle.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"

void ATripoHUD::ShowGiftReceipt(int32 AbilityIndex, int32 Level, float RevealDelay, bool bRandomDraw)
{
    if (bGiftReceipt || AbilityIndex < INDEX_NONE || AbilityIndex >= 8) return;
    GiftAbility=AbilityIndex;
    GiftLevel=Level;
    bGiftSpin=bRandomDraw && AbilityIndex!=INDEX_NONE;
    GiftSpinStart=FPlatformTime::Seconds()+FMath::Max(0.f,RevealDelay);
    GiftRevealStart=GiftSpinStart+(bGiftSpin?3.6:0.0);
    bGiftReceipt=true;
    if (auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) R->SetPauseReason(ETripoPauseReason::Reward,true);
    PanelKey.Empty();
}
TSharedRef<SWidget> ATripoHUD::BuildGiftReceipt()
{
    static const TCHAR* Names[]={TEXT("平面位移"),TEXT("上位移"),TEXT("蹬墙跳"),TEXT("垫脚石"),TEXT("局部减慢"),TEXT("时回"),TEXT("分身"),TEXT("奖励加时")};
    static const TCHAR* Descriptions[]={TEXT("向前冲出一段距离，跨过眼前的阻碍。"),TEXT("在空中再次跃起，延续向前的惯性。"),TEXT("借助墙面发力，跃向更高的地方。"),TEXT("放下一块临时落脚点，为自己铺路。"),TEXT("放慢附近的时间，留出从容应对的余地。"),TEXT("回到片刻之前，重新选择下一步。"),TEXT("留下分身，在不同位置之间切换操控。"),TEXT("为限时挑战争取更多时间。")};
    const bool bKeepsake=GiftAbility==INDEX_NONE;
    const TWeakObjectPtr<ATripoHUD> WeakThis(this);
    auto Column=SNew(SVerticalBox);
    Column->AddSlot().AutoHeight().HAlign(HAlign_Center)[TripoMenu::Label(TEXT("一  来自明天的礼物  一"),17,TripoMenu::Brass,true)];
    Column->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,8,0,4)
        [TripoMenu::Label(bKeepsake?TEXT("收藏这一份心意"):GiftLevel==1?TEXT("新的能力已获得"):TEXT("能力成长了"),27,TripoMenu::Ink,true)];
    if (!bKeepsake)
    {
        auto Icon=SNew(SImage).Image(&SkillBrushes[GiftAbility]);
        Icon->SetRenderTransformPivot(FVector2D(.5,.5));
        Icon->SetRenderTransform(TAttribute<TOptional<FSlateRenderTransform>>::CreateLambda([WeakThis]() -> TOptional<FSlateRenderTransform>
        {
            const float T=WeakThis.IsValid()?float(FPlatformTime::Seconds()-WeakThis->GiftRevealStart):1.f;
            const float Scale=T<1.f? .65f+.35f*FMath::Clamp(T/.6f,0.f,1.f)+.1f*FMath::Sin(FMath::Clamp((T-.3f)/.7f,0.f,1.f)*PI):1.f;
            return FSlateRenderTransform(Scale);
        }));
        Icon->SetColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([WeakThis]()
        {
            const float T=WeakThis.IsValid()?float(FPlatformTime::Seconds()-WeakThis->GiftRevealStart):2.f;
            const float Glow=1.f+.6f*FMath::Max(0.f,1.f-FMath::Abs(T-.7f)/.4f);
            return FSlateColor(FLinearColor(Glow,Glow,Glow,FMath::Clamp(T/.5f,0.f,1.f)));
        }));
        Column->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,8)[SNew(SBox).WidthOverride(96).HeightOverride(96)[Icon]];
    }
    Column->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,6)[TripoMenu::Label(bKeepsake?TEXT("纪念礼物"):Names[GiftAbility],27,TripoMenu::Ink,true)];
    Column->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,0,0,8)
        [TripoMenu::Label(bKeepsake?TEXT("这份心意，已经收好了"):FString::Printf(TEXT("Lv.%d    →    Lv.%d"),GiftLevel-1,GiftLevel),22,TripoMenu::Brass,true)];
    Column->AddSlot().AutoHeight()[TripoMenu::Rule()];
    Column->AddSlot().AutoHeight().Padding(0,10)[TripoMenu::Label(bKeepsake?TEXT("礼物中没有可升级的能力，本次开盒已记入旅途。没有扣除任何已有能力。"):Descriptions[GiftAbility],17)];
    Column->AddSlot().AutoHeight().Padding(0,10)
        [SNew(SButton).ButtonStyle(&MenuButtonStyle()).ButtonColorAndOpacity(FLinearColor(.08,.14,.11)).HAlign(HAlign_Center).ContentPadding(FMargin(22,13))
        .IsEnabled_Lambda([WeakThis]{return WeakThis.IsValid() && FPlatformTime::Seconds()-WeakThis->GiftRevealStart>=.85;})
        .OnClicked_Lambda([WeakThis]{if(WeakThis.IsValid()) WeakThis->HandleAction(TEXT("gift.confirm"));return FReply::Handled();})
        [TripoMenu::Label(TEXT("收下礼物 · 继续旅途"),20,TripoMenu::Paper,true,false)]];
    Column->AddSlot().AutoHeight().HAlign(HAlign_Center)[TripoMenu::Label(TEXT("能力已生效 · 在安全点存档可保存本次收获"),13,TripoMenu::Muted)];
    auto Panel=FramePanel(Column,&ItemBrush,FVector2D(820,780),FMargin(155,145,125,125));
    auto ReceiptPanel=SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("NoBrush")).Padding(0)
        .ColorAndOpacity_Lambda([WeakThis]{return FLinearColor(1,1,1,WeakThis.IsValid()?FMath::Clamp(float((FPlatformTime::Seconds()-WeakThis->GiftRevealStart-.2)/.4),0.f,1.f):1.f);})[Panel];
    auto Spin=SNew(SVerticalBox);
    Spin->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,0,0,22)
        [TripoMenu::Label(TEXT("礼物正在揭晓"),22,TripoMenu::Paper,true)];
    Spin->AddSlot().AutoHeight().HAlign(HAlign_Center)
        [SNew(SBox).WidthOverride(132).HeightOverride(132)
            [SNew(SImage).Image_Lambda([WeakThis]() -> const FSlateBrush*
            {
                if (!WeakThis.IsValid()) return nullptr;
                // Integral of linearly decreasing speed: 32 icon steps in 3.2 seconds.
                // Offset the sequence so the final step lands on the committed reward.
                const double U=FMath::Clamp((FPlatformTime::Seconds()-WeakThis->GiftSpinStart)/3.2,0.0,1.0);
                const int32 Step=FMath::Min(32,FMath::FloorToInt(32.0*(2.0*U-U*U)+.000001));
                return &WeakThis->SkillBrushes[(WeakThis->GiftAbility+Step)%8];
            })]];
    Spin->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,22,0,0)
        [TripoMenu::Label(TEXT("一份新的可能，正在到来"),15,TripoMenu::Paper)];
    return SNew(SOverlay)
        +SOverlay::Slot()[ReceiptPanel]
        +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [SNew(SBox).Visibility_Lambda([WeakThis]
        {
            const double Now=FPlatformTime::Seconds();
            return WeakThis.IsValid() && WeakThis->bGiftSpin && Now>=WeakThis->GiftSpinStart && Now<WeakThis->GiftRevealStart+.2
                ?EVisibility::HitTestInvisible:EVisibility::Collapsed;
        })[Spin]];
}
