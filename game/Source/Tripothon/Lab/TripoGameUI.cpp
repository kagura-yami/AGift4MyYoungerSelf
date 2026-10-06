#include "World/TripoOfficeCipher.h"
#include "Player/TripoPlayerController.h"
#include "Lab/TripoHUD.h"
#include "Lab/TripoMenuStyle.h"
#include "Lab/TripoGuideIcon.h"
#include "Player/TripoCharacter.h"
#include "Player/TripoMovementComponent.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Engine/Texture2D.h"
#include "Abilities/TripoStone.h"
#include "Time/TripoEchoActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/SLeafWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"

// Paint the original alpha texture twice: dim base and a clockwise bright sector.
// No opaque circle/rectangle is added, so transparent edges remain transparent.
class STripoCooldownIcon : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STripoCooldownIcon) {}
        SLATE_ARGUMENT(const FSlateBrush*, Brush)
        SLATE_ATTRIBUTE(FLinearColor, Tint)
        SLATE_ATTRIBUTE(float, Progress)
        SLATE_ATTRIBUTE(double, ActionTime)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args) { Brush=Args._Brush; Tint=Args._Tint; Progress=Args._Progress; ActionTime=Args._ActionTime; SetCanTick(true); }
    virtual void Tick(const FGeometry&, double, float) override
    {
        const float Current=Progress.Get(1.f);
        const double Now=ActionTime.Get(0.);
        if (bSampled && Previous<.9999f && Current>=.9999f) CompletionAt=Now;
        if (Current<.9999f || Now<LastTime) CompletionAt=-1.;
        Previous=Current; LastTime=Now; bSampled=true;
    }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(48,48); }
    virtual bool ComputeVolatility() const override { return true; }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Allotted, const FSlateRect&, FSlateWindowElementList& Out,
        int32 Layer, const FWidgetStyle& Style, bool) const override
    {
        const float P=FMath::Clamp(Progress.Get(1.f),0.f,1.f);
        const double Age=CompletionAt>=0?ActionTime.Get(0.)-CompletionAt:1.;
        const bool bComplete=Age>=0 && Age<.32;
        const float T=bComplete?float(Age/.32):1.f;
        const float Pulse=bComplete?FMath::Sin(PI*T)*(1.f-T):0.f;
        const float Scale=1.f+.055f*Pulse;
        const FGeometry G=Allotted.MakeChild(Allotted.GetLocalSize(),FSlateLayoutTransform(Scale,Allotted.GetLocalSize()*(1.f-Scale)*.5f));
        const FLinearColor Color=Tint.Get(FLinearColor::White)*Style.GetColorAndOpacityTint();
        FLinearColor Dim=Color; Dim.R*=.23f; Dim.G*=.23f; Dim.B*=.23f;
        FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(),Brush,ESlateDrawEffect::None,P>=1.f?Color:Dim);
        if (P>=1.f && bComplete)
        {
            const FBox2f UV=Brush->GetUVRegion();
            const FVector2f Size(G.GetLocalSize());
            TArray<FSlateVertex> Shine; TArray<SlateIndex> Triangles;
            constexpr int32 Steps=16;
            for (int32 Y=0; Y<=Steps; ++Y) for (int32 X=0; X<=Steps; ++X)
            {
                const FVector2f Q(float(X)/Steps,float(Y)/Steps);
                // A soft diagonal glint samples the icon itself, preserving its alpha silhouette.
                const float Distance=FMath::Abs(Q.X+.35f*Q.Y-FMath::Lerp(-.2f,1.55f,T));
                const float Band=FMath::Square(FMath::Max(0.f,1.f-Distance/.18f));
                const FLinearColor Glint(1.f,.94f,.72f,Band*.85f*(1.f-T)*Style.GetColorAndOpacityTint().A);
                Shine.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),Q*Size,
                    UV.Min+Q*(UV.Max-UV.Min),Glint.ToFColor(false)));
            }
            for (int32 Y=0; Y<Steps; ++Y) for (int32 X=0; X<Steps; ++X)
            {
                const int32 A=Y*(Steps+1)+X, B=A+Steps+1;
                for (int32 Index : {A,A+1,B,A+1,B+1,B}) Triangles.Add(Index);
            }
            FSlateDrawElement::MakeCustomVerts(Out,Layer+1,FSlateApplication::Get().GetRenderer()->GetResourceHandle(*Brush),Shine,Triangles,nullptr,0,0);
            // Two restrained brass rim accents expand and fade, without an opaque background.
            for (int32 Arc=0; Arc<2; ++Arc)
            {
                TArray<FVector2D> Points;
                for (int32 Step=0; Step<=24; ++Step)
                {
                    const float Angle=Arc*PI-.8f+1.6f*Step/24.f;
                    Points.Add(FVector2D(Size)*.5+FVector2D(FMath::Cos(Angle),FMath::Sin(Angle))*Size.X*(.46f+.055f*T));
                }
                FSlateDrawElement::MakeLines(Out,Layer+2,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,
                    FLinearColor(1.f,.76f,.3f,Pulse*.9f),true,1.3f);
            }
            return Layer+2;
        }
        if (P<=0.f || P>=1.f) return Layer;
        TArray<FSlateVertex> V; TArray<SlateIndex> Indices;
        const FBox2f UV=Brush->GetUVRegion();
        const FVector2f Size(G.GetLocalSize());
        auto Vertex=[&](FVector2f Q)
        {
            V.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),Q*Size,
                UV.Min+Q*(UV.Max-UV.Min),Color.ToFColor(false)));
        };
        Vertex(FVector2f(.5f,.5f));
        // Include every square corner exactly to avoid clipping the badge ornament.
        TArray<float> Angles; Angles.Add(0.f);
        const float End=P*2.f*PI;
        for (int32 Step=1; Step<=128; ++Step)
        {
            const float A=Step*2.f*PI/128.f;
            if (A>=End) break;
            Angles.Add(A);
        }
        Angles.Add(End);
        for (float A:Angles)
        {
            const FVector2f D(FMath::Sin(A),-FMath::Cos(A));
            Vertex(FVector2f(.5f,.5f)+D*(.5f/FMath::Max(FMath::Abs(D.X),FMath::Abs(D.Y))));
        }
        for (int32 I=1; I<V.Num()-1; ++I) { Indices.Add(0); Indices.Add(I); Indices.Add(I+1); }
        const auto Handle=FSlateApplication::Get().GetRenderer()->GetResourceHandle(*Brush);
        FSlateDrawElement::MakeCustomVerts(Out,Layer+1,Handle,V,Indices,nullptr,0,0);
        return Layer+1;
    }
private:
    const FSlateBrush* Brush=nullptr;
    TAttribute<FLinearColor> Tint;
    TAttribute<float> Progress;
    TAttribute<double> ActionTime;
    double CompletionAt=-1., LastTime=0.;
    float Previous=1.f;
    bool bSampled=false;
};

namespace TripoUI
{
const TCHAR* Names[] = {TEXT("平面位移"),TEXT("上位移"),TEXT("蹬墙跳"),TEXT("垫脚石"),TEXT("局部减慢"),TEXT("时回"),TEXT("分身"),TEXT("奖励加时")};
const TCHAR* Keys[] = {TEXT("Shift"),TEXT("Ctrl"),TEXT("Space"),TEXT("Q"),TEXT("F"),TEXT("R / T"),TEXT("C"),TEXT("被动")};
const TCHAR* Help[] = {
TEXT("沿移动方向冲刺，无输入时沿角色前方。空中次数与冷却分别计算，落地恢复次数。"),
TEXT("保留水平惯性，施加向上初速度后自然受重力影响。空中次数用完需落地恢复。"),
TEXT("空中面向可蹬跳墙面，按 Space 向外和向上弹跳。同一墙面附近不能反复蹬跳。"),
TEXT("按住 Q 预览，滚轮调节远近，松开放置。位置受阻、禁建区域或达到数量上限时不能放置。"),
TEXT("减慢前方 6 米内可受时间影响的平台。目标不能被遮挡。"),
TEXT("R 回溯自身，T 回溯平台，共享等级与冷却。需要足够历史，遇阻挡可能提前结束。"),
TEXT("V 原地创建；短按 C 顺序切换，长按 C 用轮盘选择。短按 X 回收最新分身；长按 X 选择回收。悬停预览，松开确认。"),
TEXT("开始挑战时自动增加奖励时间预算，无需主动施放。")};
const FLinearColor Ink(.035f,.065f,.045f);
const FLinearColor Cream(.91f,.85f,.68f);
const FLinearColor Green(.055f,.105f,.075f);
}

void ATripoHUD::InitializeUIArt()
{
    auto Load = [this](const TCHAR* Path)
    {
        auto* Texture = LoadObject<UTexture2D>(nullptr, Path);
        if (Texture) UITextures.Add(Texture);
        return Texture;
    };
    auto* Icons = Load(TEXT("/Game/UI/Prototype/T_UI_SkillIconsAlpha.T_UI_SkillIconsAlpha"));
    for (int32 I=0; I<8; ++I)
    {
        auto& B = SkillBrushes[I];
        B.SetResourceObject(Icons); B.DrawAs = ESlateBrushDrawType::Image; B.ImageSize = FVector2D(64,64);
        // Inset each cell slightly to avoid sampling the neighbouring badge.
        B.SetUVRegion(FBox2f(FVector2f((I%4+.002f)/4.f,(I/4+.002f)/2.f),FVector2f((I%4+.998f)/4.f,(I/4+.998f)/2.f)));
    }
    PauseBrush.SetResourceObject(Load(TEXT("/Game/UI/Prototype/T_UI_PauseAlpha.T_UI_PauseAlpha")));
    PauseBrush.DrawAs = ESlateBrushDrawType::Image; PauseBrush.ImageSize = FVector2D(480,720);
    BookBrush.SetResourceObject(Load(TEXT("/Game/UI/Prototype/T_UI_BookAlpha.T_UI_BookAlpha")));
    BookBrush.DrawAs=ESlateBrushDrawType::Image; BookBrush.ImageSize=FVector2D(960,640);
    auto Panel=[&](FSlateBrush& Brush,const TCHAR* Path,FVector2D Size)
    { Brush.SetResourceObject(Load(Path)); Brush.DrawAs=ESlateBrushDrawType::Image; Brush.ImageSize=Size; };
    Panel(MainMenuArtBrush,TEXT("/Game/UI/FrontEnd/T_MainMenuDesk.T_MainMenuDesk"),FVector2D(1920,1080));
    Panel(PaperBrush,TEXT("/Game/UI/Prototype/T_UI_PaperAlpha.T_UI_PaperAlpha"),FVector2D(840,630));
    Panel(UpgradeBrush,TEXT("/Game/UI/Prototype/T_UI_UpgradeAlpha.T_UI_UpgradeAlpha"),FVector2D(960,720));
    Panel(ItemBrush,TEXT("/Game/UI/Prototype/T_UI_ItemAlpha.T_UI_ItemAlpha"),FVector2D(840,630));
    Panel(DialogueBrush,TEXT("/Game/UI/Prototype/T_UI_DialogueAlpha.T_UI_DialogueAlpha"),FVector2D(1080,450));
}
void ATripoHUD::UpdateStoneIndicator()
{
    const auto* C=Player();
    auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    FTripoAbilityParameters P;
    if (!C || !R || !C->Abilities->GetParameters(ETripoAbility::StepStone,P))
    {
        StoneCount=StoneCapacity=0; StoneWait=StoneRecoveryUntil=0;
        bStoneIndicatorInitialized=false; return;
    }
    int32 Count=0;
    double Soonest=TNumericLimits<double>::Max(), Lifetime=1;
    for (TActorIterator<ATripoStone> It(GetWorld()); It; ++It)
    {
        if (It->GetOwner()!=C || !It->IsUsable()) continue;
        ++Count;
        const double Left=It->GetRemainingLifetime();
        if (Left<Soonest) { Soonest=Left; Lifetime=FMath::Max(double(It->Duration),.001); }
    }
    const bool bWasFull=StoneCapacity>0 && StoneCount>=StoneCapacity;
    const bool bFull=P.Capacity>0 && Count>=P.Capacity;
    // Recovered capacity gets a brief dark-to-ready transition; placement uses real cooldown.
    if (bStoneIndicatorInitialized && !bFull && bWasFull)
        StoneRecoveryUntil=R->GetActionSeconds()+.1;
    if (bFull) StoneRecoveryUntil=0;
    StoneCount=Count; StoneCapacity=P.Capacity;
    StoneWait=bFull?Soonest:0; StoneLifetime=Lifetime;
    bStoneIndicatorInitialized=true;
}
FText ATripoHUD::SkillStatus(int32 Index) const
{
    const auto* C = Player(); if (!C) return FText::GetEmpty();
    const auto Id = ETripoAbility(Index);
    if (!C->Abilities->GetLevel(Id)) return FText::FromString(TEXT("未解锁"));
    if (Id == ETripoAbility::StepStone)
    {
        return FText::FromString(FString::Printf(TEXT("%d / %d"),StoneCount,StoneCapacity));
    }
    if (Id==ETripoAbility::Echo)
        if (const auto* PC=Cast<ATripoPlayerController>(PlayerOwner))
            return FText::FromString(FString::Printf(TEXT("%d/%d · V 创建 · C 切换 · X 回收"),PC->GetEchoCount(),PC->GetEchoCapacity()));
    const double Cooldown = C->Abilities->GetCooldownRemaining(Id);
    const auto* M = Cast<UTripoMovementComponent>(C->GetCharacterMovement());
    const bool bSpent = M && ((Index == 0 && M->bDashSpent) || (Index == 1 && M->bUpSpent));
    if (Cooldown > 0) return FText::FromString(FString::Printf(TEXT("%.1fs%s"),Cooldown,bSpent ? TEXT(" · 落地") : TEXT("")));
    if (bSpent) return FText::FromString(TEXT("落地恢复"));
    if (C->Abilities->HasActiveAbility(Id)) return FText::FromString(TEXT("生效中"));
    return FText::FromString(TEXT("就绪"));
}
FText ATripoHUD::ChallengeText() const
{
    const auto* C = Player(); auto* P = UTripoProgressSubsystem::Get(this);
    if (!C || !P || P->GetPhase() != ETripoChallengePhase::Running) return FText::GetEmpty();
    return FText::FromString(FString::Printf(TEXT("挑战  %.1f / %.1f 秒\n奖励加时 +%.0f 秒 · %s"),P->GetElapsed(),P->GetBudget(),C->Abilities->GetBonusTimeSeconds(),P->IsChoiceEligible() && P->GetElapsed() <= P->GetBudget() ? TEXT("限时内可自选") : TEXT("超时仍可继续")));
}
TSharedRef<SWidget> ATripoHUD::BuildSkillBar()
{
    const TWeakObjectPtr<ATripoHUD> Weak(this);
    auto Row = SNew(SVerticalBox);
    // Static brushes outlive the Slate widgets; the HUD retains the icon textures.
    static const FSlateRoundedBoxBrush KeyBase(FLinearColor(.12f,.14f,.13f), 5.f);
    static const FSlateRoundedBoxBrush KeyFace(FLinearColor(.88f,.86f,.77f), 4.f);
    auto Keycap = [](const TCHAR* Label) -> TSharedRef<SWidget>
    {
        return SNew(SBox).MinDesiredWidth(34).HeightOverride(31)
            [SNew(SBorder).BorderImage(&KeyBase).Padding(FMargin(1,1,1,4))
                [SNew(SBorder).BorderImage(&KeyFace).Padding(FMargin(7,2)).HAlign(HAlign_Center).VAlign(VAlign_Center)
                    [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold",13)).ColorAndOpacity(TripoUI::Ink).Text(FText::FromString(Label))]]];
    };
    for (int32 I=0; I<7; ++I)
    {
        auto Keys = SNew(SHorizontalBox);
        Keys->AddSlot().AutoWidth().VAlign(VAlign_Center)[Keycap(I==5 ? TEXT("R") : TripoUI::Keys[I])];
        if (I==5) Keys->AddSlot().AutoWidth().Padding(5,0).VAlign(VAlign_Center)[Keycap(TEXT("T"))];
        Row->AddSlot().AutoHeight().Padding(0,3)
        [SNew(SHorizontalBox)
            .Visibility_Lambda([Weak,I] { return Weak.IsValid() && Weak->Player() && Weak->Player()->Abilities->GetLevel(ETripoAbility(I))>0 ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [SNew(SBox).WidthOverride(48).HeightOverride(48)
                [SNew(STripoCooldownIcon).Brush(&SkillBrushes[I]).ActionTime_Lambda([Weak]
                {
                    auto* R=Weak.IsValid()?UTripoRuntimeSubsystem::GetRuntime(Weak.Get()):nullptr;
                    return R?R->GetActionSeconds():0.;
                }).Progress_Lambda([Weak,I]
                {
                    auto* C=Weak.IsValid()?Weak->Player():nullptr;
                    FTripoAbilityParameters P;
                    if (!C || !C->Abilities->GetParameters(ETripoAbility(I),P) || P.Cooldown<=0) return 1.f;
                    const float Ready=float(1.-FMath::Clamp(C->Abilities->GetCooldownRemaining(ETripoAbility(I))/P.Cooldown,0.,1.));
                    if (I==3)
                    {
                        if (Weak->StoneCapacity>0 && Weak->StoneCount>=Weak->StoneCapacity)
                            return FMath::Min(Ready,float(1.-FMath::Clamp(Weak->StoneWait/Weak->StoneLifetime,0.,1.)));
                        auto* R=UTripoRuntimeSubsystem::GetRuntime(Weak.Get());
                        const float Recovery=R?float(1.-FMath::Clamp((Weak->StoneRecoveryUntil-R->GetActionSeconds())/.1,0.,1.)):1.f;
                        return FMath::Min(Ready,Recovery);
                    }
                    return Ready;
                }).Tint_Lambda([Weak,I]
                {
                    const int32 Level = Weak.IsValid() && Weak->Player() ? Weak->Player()->Abilities->GetLevel(ETripoAbility(I)) : 0;
                    // Ivory, jade and amber retain the illustrated badge shading.
                    return Level>=3 ? FLinearColor(1.f,.63f,.16f) : Level==2 ? FLinearColor(.35f,1.f,.66f) : FLinearColor(.95f,.95f,.90f);
                })]]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8,0)[Keys]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4,0)
            [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold",12)).ColorAndOpacity(TripoUI::Cream)
                .ShadowOffset(FVector2D(1,1)).ShadowColorAndOpacity(FLinearColor::Black)
                .Text_Lambda([Weak,I]
                {
                    if (!Weak.IsValid()) return FText::GetEmpty();
                    if (I!=3 && !(I==6 && Weak->Player() && Weak->Player()->Abilities->HasActiveAbility(ETripoAbility::Echo)) && Weak->Player() && Weak->Player()->Abilities->GetCooldownRemaining(ETripoAbility(I))>0) return FText::GetEmpty();
                    const FText State=Weak->SkillStatus(I);
                    return State.EqualTo(FText::FromString(TEXT("就绪"))) ? FText::GetEmpty() : State;
                })]
        ];
    }
    return Row;
}
bool ATripoHUD::NavigateBack()
{
    if (bStoryMovie) { StopStoryMovie(); return true; }
    if (OfficeCipher.IsValid()) { CloseOfficeCipher(); return true; }
    if (bGiftReceipt) { HandleAction(TEXT("gift.confirm")); return true; }
    if (bEntryMenu && bConfirmNewGame) { bConfirmNewGame=false; PanelKey.Empty(); return true; }
    if (!bEntryMenu && (!PlayerOwner || !PlayerOwner->IsPaused())) return false;
    if (bCollection) bCollection=false;
    else if (bAbilityConfig) bAbilityConfig=false;
    else if (MenuPage == EMenuPage::Handbook) MenuPage=EMenuPage::Tutorial;
    else if (MenuPage == EMenuPage::Tutorial || MenuPage == EMenuPage::Preferences) MenuPage=EMenuPage::Settings;
    else if (MenuPage != EMenuPage::Pause) MenuPage=EMenuPage::Pause;
    else return false;
    PanelKey.Empty(); return true;
}
const FButtonStyle& ATripoHUD::MenuButtonStyle()
{
    static const FButtonStyle Style=FButtonStyle()
        .SetNormal(FSlateRoundedBoxBrush(FLinearColor::White,8.f))
        .SetHovered(FSlateRoundedBoxBrush(FLinearColor(1.25f,1.2f,1.1f),8.f))
        .SetPressed(FSlateRoundedBoxBrush(FLinearColor(.72f,.8f,.72f),8.f));
    return Style;
}
TSharedRef<SWidget> ATripoHUD::FramePanel(TSharedRef<SWidget> Content,const FSlateBrush* Art,FVector2D Size,FMargin Padding)
{
    // The image owns the complete silhouette. No solid border or cropped paper texture behind it.
    return SNew(SBox).WidthOverride(Size.X).HeightOverride(Size.Y)
        [SNew(SOverlay)
            + SOverlay::Slot()[SNew(SImage).Image(Art).Visibility(EVisibility::HitTestInvisible)]
            + SOverlay::Slot().Padding(Padding)[Content]];
}
TSharedRef<SWidget> ATripoHUD::BuildMenuPage()
{
    using namespace TripoUI;
    const TWeakObjectPtr<ATripoHUD> Weak(this);
    struct FPaperMotion { float Hover=0,Press=0,Flash=0; double Trigger=-1; };
    const auto Pending=MakeShared<bool>(false);
    auto Animate=[Weak,Pending](TSharedRef<SButton> B,const FString& Action)
    {
        const auto Motion=MakeShared<FPaperMotion>();
        const TWeakPtr<SButton> ButtonWeak=B;
        static const FSlateRoundedBoxBrush Glaze(FLinearColor::White,8.f);
        auto Original=B->GetContent();
        B->SetContent(SNew(SOverlay)
            +SOverlay::Slot()[Original]
            +SOverlay::Slot()[SNew(SBorder).Padding(0).BorderImage(&Glaze)
                .Visibility(EVisibility::HitTestInvisible)
                .BorderBackgroundColor_Lambda([Motion]{return FLinearColor(1,.77f,.35f,.07f*Motion->Hover+.22f*Motion->Flash);})]
            +SOverlay::Slot().VAlign(VAlign_Bottom).Padding(0,0,0,-4)
                [SNew(SBox).HeightOverride(1)
                    [SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                        .Visibility(EVisibility::HitTestInvisible)
                        .BorderBackgroundColor_Lambda([Motion]{return FLinearColor(.48f,.29f,.1f,.45f*Motion->Hover+.3f*Motion->Flash);})]]);
        B->SetRenderTransformPivot(FVector2D(.5f,.5f));
        B->SetOnClicked(FOnClicked::CreateLambda([Motion,Pending]{
            if(!*Pending) { *Pending=true; Motion->Trigger=FPlatformTime::Seconds(); }
            return FReply::Handled();
        }));
        B->RegisterActiveTimer(0,FWidgetActiveTimerDelegate::CreateLambda(
            [ButtonWeak,Motion,Pending,Weak,Action](double,float Dt){
                const auto P=ButtonWeak.Pin();
                if(!P.IsValid()) return EActiveTimerReturnType::Stop;
                const bool Hot=P->IsEnabled() && (P->IsHovered() || P->HasKeyboardFocus());
                Motion->Hover=FMath::FInterpTo(Motion->Hover,Hot?1.f:0.f,Dt,12.f);
                Motion->Press=FMath::FInterpTo(Motion->Press,P->IsPressed()?1.f:0.f,Dt,26.f);
                if(Motion->Trigger>=0) {
                    const float Age=float(FPlatformTime::Seconds()-Motion->Trigger);
                    Motion->Flash=FMath::Sin(FMath::Clamp(Age/.14f,0.f,1.f)*PI);
                    if(Age>=.14f) {
                        Motion->Trigger=-1; Motion->Flash=0; *Pending=false;
                        if(Weak.IsValid()) Weak->HandleAction(Action);
                        return EActiveTimerReturnType::Continue;
                    }
                }
                P->SetRenderTransform(FSlateRenderTransform(FScale2D(1+.006f*Motion->Hover-.018f*Motion->Press),
                    FVector2D(0,-2.f*Motion->Hover+3.f*Motion->Press)));
                P->Invalidate(EInvalidateWidgetReason::Paint);
                return EActiveTimerReturnType::Continue;
            }));
        return B;
    };
    auto Button = [Weak,Animate](const FString& Label,const FString& Action,bool bPrimary=false)
    {
        return Animate(SNew(SButton).ButtonStyle(&MenuButtonStyle())
            .ButtonColorAndOpacity(bPrimary?Green:FLinearColor(.22f,.28f,.20f,.10f))
            .ContentPadding(FMargin(16,8)).HAlign(HAlign_Left)
            .OnClicked_Lambda([Weak,Action] { if (Weak.IsValid()) Weak->HandleAction(Action); return FReply::Handled(); })
            [TripoMenu::Label(Label,18,bPrimary?TripoMenu::Paper:Ink,true,false)],Action);
    };
    if (MenuPage == EMenuPage::Pause)
    {
        auto Buttons=SNew(SVerticalBox);
        const TCHAR* Labels[]={TEXT("继续游戏"),TEXT("保存安全进度"),TEXT("读取安全存档"),TEXT("设置与教程"),TEXT("更多选项"),TEXT("退出游戏")};
        const TCHAR* Actions[]={TEXT("resume"),TEXT("save"),TEXT("load"),TEXT("ui.settings"),TEXT("ui.more"),TEXT("quit")};
        for (int32 I=0; I<6; ++I)
            Buttons->AddSlot().FillHeight(1).Padding(0,I==3?10:2,0,2)[Button(Labels[I],Actions[I],I==0)];
        return SNew(SBox).WidthOverride(440).HeightOverride(660)
            [SNew(SOverlay)
                + SOverlay::Slot()[SNew(SImage).Image(&PauseBrush)]
                + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(12,66,0,0)
                    [TripoMenu::Label(TEXT("稍作停留"),28,Ink,true)]
                + SOverlay::Slot().Padding(88,134,66,202)[Buttons]
                + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(151,464,62,0)
                    [SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[TripoMenu::Label(TEXT("P  返回旅途"),13,TripoMenu::Brass,false,false)]
                        + SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[TripoMenu::Label(TEXT("读档从安全进度开始"),11,TripoMenu::Muted,false,false)]]];
    }
    if (MenuPage == EMenuPage::Handbook)
    {
        const int32 I=HandbookAbility;
        const int32 Level=Player()?Player()->Abilities->GetLevel(ETripoAbility(I)):0;
        auto Entries=SNew(SVerticalBox);
        for (int32 Index=0; Index<8; ++Index)
        {
            const bool bSelected=Index==I;
            Entries->AddSlot().FillHeight(1).Padding(0,2)
                [SNew(SButton).ButtonStyle(&MenuButtonStyle())
                    .ButtonColorAndOpacity(bSelected?Green:FLinearColor(0,0,0,0)).ContentPadding(FMargin(6,3))
                    .OnClicked_Lambda([Weak,Index]{if(Weak.IsValid()) Weak->HandleAction(FString::Printf(TEXT("ui.skill.%d"),Index));return FReply::Handled();})
                    [SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SBox).WidthOverride(34).HeightOverride(34)[SNew(SImage).Image(&SkillBrushes[Index])]]
                        + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(9,0)[TripoMenu::Label(Names[Index],17,bSelected?TripoMenu::Paper:Ink)]]];
        }
        auto Detail=SNew(SVerticalBox);
        Detail->AddSlot().AutoHeight()[TripoMenu::Label(FString::Printf(TEXT("能力 %02d   /   %s"),I+1,Level?TEXT("已获得"):TEXT("未解锁")),13,TripoMenu::Brass)];
        Detail->AddSlot().AutoHeight().Padding(0,9,110,0)[TripoMenu::Label(Names[I],38,Ink,true)];
        Detail->AddSlot().AutoHeight().Padding(0,12,0,0)
            [SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()[TripoMenu::Keycap(Keys[I])]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(14,0)[TripoMenu::Label(FString::Printf(TEXT("等级  %d / 3"),Level),15,TripoMenu::Muted)]];
        Detail->AddSlot().AutoHeight().Padding(0,18,0,18)[TripoMenu::Rule()];
        Detail->AddSlot().AutoHeight()[TripoMenu::Label(Help[I],18)];
        FTripoAbilityParameters Params;
        FString Value;
        if (Player() && Player()->Abilities->GetParameters(ETripoAbility(I),Params))
        {
            switch(I)
            {
            case 0: Value=FString::Printf(TEXT("位移距离   %.2f 米"),Params.Distance/100); break;
            case 1: Value=TEXT("保留水平惯性 · 次数落地恢复"); break;
            case 2: Value=TEXT("需要有效墙面"); break;
            case 3: Value=FString::Printf(TEXT("同时存在   %d 块     /     存续   %.1f 秒"),Params.Capacity,Params.Duration); break;
            case 4: Value=FString::Printf(TEXT("目标速度   %.0f%%     /     持续   %.1f 秒"),Params.Strength*100,Params.Duration); break;
            case 5: Value=FString::Printf(TEXT("最多回溯   %.1f 秒"),Params.Duration); break;
            case 6: Value=FString::Printf(TEXT("数量上限   %d 个     /     持续存在"),Params.Capacity); break;
            case 7: Value=FString::Printf(TEXT("奖励时间   +%.0f 秒"),Params.Strength); break;
            }
            if (I!=7 && Params.Cooldown>0) Value+=FString::Printf(TEXT("\n冷却时间   %.1f 秒"),Params.Cooldown);
        }
        else Value=TEXT("获得能力后，在这里查看当前等级的效果。");
        Detail->AddSlot().AutoHeight().Padding(0,20,0,5)[TripoMenu::Label(TEXT("当前效果"),13,TripoMenu::Brass)];
        Detail->AddSlot().AutoHeight()[TripoMenu::Label(Value,16,TripoMenu::Muted)];
        return SNew(SBox).WidthOverride(960).HeightOverride(640)
            [SNew(SOverlay)
                + SOverlay::Slot()[SNew(SImage).Image(&BookBrush)]
                + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(83,58,0,0)[TripoMenu::Label(TEXT("能力手册"),23,Ink,true)]
                + SOverlay::Slot().Padding(72,111,654,106)[Entries]
                + SOverlay::Slot().Padding(392,90,72,143)[SNew(SScrollBox).AnimateWheelScrolling(true).WheelScrollMultiplier(0.7f).ConsumeMouseWheel(EConsumeMouseWheel::Always)+SScrollBox::Slot()[Detail]]
                + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(777,94,0,0)
                    [SNew(SBox).WidthOverride(80).HeightOverride(80)[SNew(SImage).Image(&SkillBrushes[I]).Visibility(EVisibility::HitTestInvisible)]]
                + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(0,0,78,87)[Button(TEXT("返回教程"),TEXT("ui.back"))]];
    }
    auto Content=SNew(SVerticalBox);
    auto Add=[&](const FString& Label,const FString& Action,bool bPrimary=false)
    { Content->AddSlot().AutoHeight().Padding(0,4)[Button(Label,Action,bPrimary)]; };
    if (MenuPage == EMenuPage::Settings)
    {
        Content->AddSlot().AutoHeight()[TripoMenu::Heading(TEXT("设置与教程"),TEXT("调整游玩偏好，或翻阅旅途指南。"))];
        auto GuideEntry=[&](int32 Index,const TCHAR* Title,const TCHAR* Description,const TCHAR* Action)
        {
            const FString Command(Action);
            Content->AddSlot().AutoHeight().Padding(0,2)
                [Animate(SNew(SButton).ButtonStyle(&MenuButtonStyle()).ButtonColorAndOpacity(FLinearColor(.25f,.28f,.17f,.025f))
                    .ContentPadding(FMargin(12,0))
                    .OnClicked_Lambda([Weak,Command]{if(Weak.IsValid()) Weak->HandleAction(Command); return FReply::Handled();})
                    [SNew(SBox).HeightOverride(86)
                    [SNew(SHorizontalBox).Visibility(EVisibility::HitTestInvisible)
                        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                            [SNew(SBox).WidthOverride(72).HeightOverride(72)[SNew(STripoGuideIcon).Kind(Index)]]
                        +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(24,0,8,0)
                            [SNew(SVerticalBox)
                                +SVerticalBox::Slot().AutoHeight()[TripoMenu::Label(Title,23,Ink,true)]
                                +SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)[TripoMenu::Label(Description,13,TripoMenu::Muted)]]
                        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8,0,18,0)
                            [TripoMenu::Label(FString::Printf(TEXT("0%d"),Index+1),12,TripoMenu::Brass,false,false)]
                        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                            [TripoMenu::Label(TEXT("›"),26,TripoMenu::Brass,true)]]],Command)];
            if(Index<2) Content->AddSlot().AutoHeight().Padding(108,3,12,3)[TripoMenu::Rule()];
        };
        GuideEntry(0,TEXT("游戏设置"),TEXT("调整声音、视角与画面"),TEXT("ui.preferences"));
        GuideEntry(1,TEXT("操作指南"),TEXT("熟悉移动、跳跃与交互"),TEXT("ui.tutorial"));
        GuideEntry(2,TEXT("能力手册"),TEXT("查阅能力效果、等级与使用条件"),TEXT("ui.handbook"));
    }
    else if (MenuPage == EMenuPage::Preferences)
    {
        Content->AddSlot().AutoHeight()[BuildSettingsControls()];
    }
    else if (MenuPage == EMenuPage::Tutorial)
    {
        Content->AddSlot().AutoHeight()[TripoMenu::Heading(TEXT("操作与教程"),TEXT("能力解锁后可用；详细条件与等级效果见能力手册。"))];
        auto Controls=[&](const TCHAR* Key,const TCHAR* Description)
        {
            Content->AddSlot().AutoHeight().Padding(0,2)
                [SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(166)[TripoMenu::Keycap(Key)]]
                    +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(16,0)[TripoMenu::Label(Description,16)]];
        };
        Controls(TEXT("W A S D / 鼠标"),TEXT("移动 / 观察"));
        Controls(TEXT("Space / E / P"),TEXT("跳跃与蹬墙 / 交互 / 暂停"));
        Controls(TEXT("Shift / Ctrl"),TEXT("平面位移 / 空中再次跃起"));
        Controls(TEXT("Q + 滚轮"),TEXT("按住预览、滚轮调距、松开放置垫脚石"));
        Controls(TEXT("F / R / T"),TEXT("减慢目标 / 回溯自身 / 回溯平台"));
        Controls(TEXT("C"),TEXT("短按顺序切换 · 长按轮盘选择"));
        Controls(TEXT("V"),TEXT("原地创建分身"));
        Controls(TEXT("X"),TEXT("短按回收最新分身 · 长按选择回收"));
    }
    else
    {
        Content->AddSlot().AutoHeight()[TripoMenu::Heading(TEXT("更多选项"),TEXT("整理旅途中的收获，或换个地方练习。"))];
        Add(TEXT("纪念物与回信"),TEXT("collection"),true);
        auto* Progress=UTripoProgressSubsystem::Get(this);
        Add(Progress->IsLabWorld(this)?TEXT("返回故事存档"):TEXT("保存并进入练习场"),TEXT("practice"));
        if (Progress->IsLabWorld(this)) Add(TEXT("练习：解锁全部三级能力"),TEXT("practice.unlock"));
        if (CanConfigureAbilities()) Add(TEXT("测试：能力配置"),TEXT("abilities.open"));
        Content->AddSlot().AutoHeight().Padding(0,16,0,8)[TripoMenu::Rule()];
        Add(TEXT("重新开始新游戏"),TEXT("new"));
        Add(TEXT("返回开始菜单（未保存进度将丢失）"),TEXT("front.return"));
    }
    auto Footer=SNew(SHorizontalBox);
    if (MenuPage==EMenuPage::Tutorial)
        Footer->AddSlot().AutoWidth()[Button(TEXT("翻阅能力手册"),TEXT("ui.handbook"),true)];
    Footer->AddSlot().FillWidth(1).HAlign(HAlign_Right)[Button(TEXT("返回上一级"),TEXT("ui.back"))];
    auto Page=SNew(SVerticalBox)
        +SVerticalBox::Slot().FillHeight(1)[SNew(SScrollBox).AnimateWheelScrolling(true).WheelScrollMultiplier(0.7f).ConsumeMouseWheel(EConsumeMouseWheel::Always)+SScrollBox::Slot()[Content]]
        +SVerticalBox::Slot().AutoHeight().Padding(0,12,0,0)[Footer];
    return FramePanel(Page,&PaperBrush,FVector2D(960,720),(MenuPage==EMenuPage::Preferences ? FMargin(120,90,120,82) : FMargin(120,105,120,98)));
}
