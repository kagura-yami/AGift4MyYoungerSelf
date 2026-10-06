#include "Player/TripoPlayerSettings.h"
#include "Lab/TripoHUD.h"
#include "Lab/TripoMenuStyle.h"
#include "Player/TripoCharacter.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Story/TripoStorySubsystem.h"
#include "World/TripoZone.h"
#include "World/TripoChapterGift.h"
#include "World/TripoOfficeCipher.h"
#include "World/TripoWorldSubsystem.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Images/SImage.h"
#include "Styling/CoreStyle.h"

static const TCHAR* AbilityNames[] = {TEXT("平面位移"),TEXT("上位移"),TEXT("蹬墙跳"),TEXT("垫脚石"),TEXT("局部减慢"),TEXT("时回"),TEXT("分身"),TEXT("奖励加时")};
ATripoCharacter* ATripoHUD::Player() const { return Cast<ATripoCharacter>(GetOwningPawn()); }
bool ATripoHUD::CanConfigureAbilities() const
{
    // The shared controller binds F2 in every gameplay map, including new chapters.
    return GetWorld() && !bFrontEnd && IsValid(Player());
}
void ATripoHUD::BeginPlay()
{
    Super::BeginPlay(); const TWeakObjectPtr<ATripoHUD> WeakThis(this);
    auto* Progress = UTripoProgressSubsystem::Get(this);
    const bool bSkipEntryMenu = GIsEditor && bSkipEntryMenuInEditor;
    bFrontEnd = UGameplayStatics::GetCurrentLevelName(this,true)==TEXT("L_MainMenu");
    bEntryMenu = bFrontEnd || (!bSkipEntryMenu && !Progress->IsLabWorld(this) && Progress->ConsumeEntryMenu());
    if (bFrontEnd) Progress->ConsumeEntryMenu();
    InitializeUIArt();
    UTripoPlayerSettings::Get()->ApplyAudio(this);
    RootWidget = SNew(SOverlay)
    + SOverlay::Slot()
    [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor(.018f,.035f,.029f,1.f))
        .Visibility_Lambda([WeakThis]
        {
            if (!WeakThis.IsValid()) return EVisibility::Collapsed;
            // Reward pauses freeze gameplay without covering the scene behind the floating panel.
            const auto* Progress = UTripoProgressSubsystem::Get(WeakThis.Get());
            const bool bReward = WeakThis->OfficeCipher.IsValid() || WeakThis->bGiftReceipt || (Progress && Progress->GetPhase() == ETripoChallengePhase::PendingReward);
            const bool bMenu = WeakThis->bEntryMenu || (!bReward && WeakThis->PlayerOwner && WeakThis->PlayerOwner->IsPaused());
            return bMenu ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
        })]
    + SOverlay::Slot()
    [SNew(SScaleBox).Stretch(EStretch::ScaleToFill)
        .Visibility_Lambda([WeakThis]{return WeakThis.IsValid() && WeakThis->bEntryMenu ? EVisibility::HitTestInvisible : EVisibility::Collapsed;})
        [SNew(SImage).Image(&MainMenuArtBrush)]]
    + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(24)
    [SNew(STextBlock).ColorAndOpacity(FLinearColor(.95,.9,.75)).ShadowOffset(FVector2D(1,1)).Font(FCoreStyle::GetDefaultFontStyle("Regular",20)).Text_Lambda([WeakThis] { return WeakThis.IsValid() && !WeakThis->IsGameplayBlocked() ? WeakThis->ChallengeText() : FText::GetEmpty(); })]
    + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(24,24,16,16)
    [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
        .Visibility_Lambda([WeakThis] { return WeakThis.IsValid() && !WeakThis->IsGameplayBlocked() ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })[BuildSkillBar()]]
    + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(32,24,32,32)
    [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
        .Visibility_Lambda([WeakThis]{return WeakThis.IsValid() && !WeakThis->IsGameplayBlocked() && WeakThis->HasInventory() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;})[BuildInventory()]]
    + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(24,24,24,110)
    [SNew(STextBlock).Font(TripoMenu::Font(20)).ColorAndOpacity(TripoMenu::Paper)
        .ShadowOffset(FVector2D(1,2)).ShadowColorAndOpacity(FLinearColor::Black)
        .Visibility_Lambda([WeakThis]{return WeakThis.IsValid() && !WeakThis->IsGameplayBlocked() ? EVisibility::HitTestInvisible:EVisibility::Collapsed;})
        .Text_Lambda([WeakThis]{return FText::FromString(WeakThis.IsValid()?WeakThis->GetPuzzleHint():FString());})]
    + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(24)
    [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)[SAssignNew(PanelHost, SBox)]]
    + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom).Padding(32,24,32,32)
    [SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)[SAssignNew(StoryHost,SBox).Visibility(EVisibility::HitTestInvisible)]]
    + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(12)
    [SNew(SScaleBox).Visibility(EVisibility::HitTestInvisible)
        .Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)[BuildEchoWheel()]];
    if (GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->AddViewportWidgetContent(RootWidget.ToSharedRef(),10);
}
void ATripoHUD::EndPlay(const EEndPlayReason::Type Reason)
{
    StopStoryMovie();
    if (RootWidget.IsValid() && GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(RootWidget.ToSharedRef());
    if (bGiftReceipt) if (auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) R->SetPauseReason(ETripoPauseReason::Reward,false);
    CloseOfficeCipher();
    RootWidget.Reset(); PanelHost.Reset(); StoryHost.Reset(); UITextures.Empty(); Super::EndPlay(Reason);
}
bool ATripoHUD::IsGameplayBlocked() const
{
    auto* P = UTripoProgressSubsystem::Get(this); auto* Story = GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    return bStoryMovie || OfficeCipher.IsValid() || bGiftReceipt || bEntryMenu || bCollection || bExchange || (PlayerOwner && PlayerOwner->IsPaused()) || (P && P->GetPhase() == ETripoChallengePhase::PendingReward);
}
FText ATripoHUD::StatusText() const
{
    const auto* C = Player(); if (!C) return FText::GetEmpty(); auto* P = UTripoProgressSubsystem::Get(this);
    if (IsGameplayBlocked()) return FText::FromString(P->GetMessage());
    FString Text = TEXT("明天寄来的礼物\nE 交互 · P 暂停 / 教程\n");
    if (CanConfigureAbilities()) Text += TEXT("F2 能力配置：自由选择能力与等级\n");
    if (!P->GetMessage().IsEmpty()) Text += TEXT("\n") + P->GetMessage();
    if (!C->Abilities->GetFailureMessage().IsEmpty()) Text += TEXT("\n") + C->Abilities->GetFailureMessage();
    if (!C->GetStonePreviewHint().IsEmpty()) Text += TEXT("\n") + C->GetStonePreviewHint();
    if (!UTripoWorldSubsystem::Get(this)->GetLastFailure().IsEmpty()) Text += TEXT("\n") + UTripoWorldSubsystem::Get(this)->GetLastFailure();
    return FText::FromString(Text);
}
void ATripoHUD::RefreshUI()
{
    UpdateStoneIndicator();
    if (!PanelHost.IsValid()) return;
    if (!bEntryMenu && (!PlayerOwner || !PlayerOwner->IsPaused())) { bAbilityConfig = false; MenuPage = EMenuPage::Pause; }
    auto* P = UTripoProgressSubsystem::Get(this); auto* Story = GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    const double Now=FPlatformTime::Seconds();
    const float StoryDt=StoryLastTick>0?FMath::Clamp(float(Now-StoryLastTick),0.f,.25f):0.f;
    StoryLastTick=Now;
    const FString StoryKey=Story->HasDialogue()?Story->GetCurrentId().ToString()+TEXT(":")+FString::FromInt(Story->GetLineIndex()):FString();
    if (TimedStoryLine!=StoryKey) { TimedStoryLine=StoryKey; StoryLineElapsed=0; }
    if (Story->HasDialogue() && !IsGameplayBlocked())
    {
        StoryLineElapsed+=StoryDt;
        const bool bAwaitChoice=Story->IsLastLine() && Story->GetCurrentId()==TEXT("Story.Finale.SendReply") && P->GetReplyChoice()==INDEX_NONE;
        const float Duration=FMath::Clamp(2.f+Story->GetLine().Len()*.16f,4.f,12.f);
        if (!bAwaitChoice && StoryLineElapsed>=Duration) { Story->AdvanceDialogue(); StoryLineElapsed=0; }
    }
    const FString Key = FString::FromInt(int32(MenuPage)) + TEXT(":") + FString::FromInt(HandbookAbility) + TEXT(":") + FString::Printf(TEXT("%d:%d:%d:%d:%d:%s"),PlayerOwner && PlayerOwner->IsPaused(),int32(P->GetPhase()),bExchange,ExchangeFrom,ExchangeTo,Story->HasDialogue() ? *Story->GetCurrent().Id.ToString() : TEXT(""));
    const FString FullKey=Key+FString::FromInt(Story->GetLineIndex());
    if (FullKey != PanelKey) { PanelKey = FullKey; RebuildPanel(); }
    const bool bModal = IsGameplayBlocked();
    if (PlayerOwner && bModal != bWasModal)
    {
        bWasModal = bModal; PlayerOwner->bShowMouseCursor = bModal;
        if (bModal) { FInputModeGameAndUI Mode; Mode.SetHideCursorDuringCapture(false); Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); PlayerOwner->SetInputMode(Mode); }
        else PlayerOwner->SetInputMode(FInputModeGameOnly());
    }
}
bool ATripoHUD::OpenExchange()
{
    auto* C = Player(); auto* P = UTripoProgressSubsystem::Get(this);
    if (!C || IsGameplayBlocked() || P->GetPhase() == ETripoChallengePhase::Running || !ATripoZone::Inside(GetWorld(),ETripoZoneKind::NPC,C->GetActorLocation())) return false;
    bExchange = true; ExchangeTransaction = FGuid::NewGuid(); PanelKey.Empty(); return true;
}
void ATripoHUD::RebuildPanel()
{
    StoryHost->SetContent(SNullWidget::NullWidget);
    if (OfficeCipher.IsValid()) { PanelHost->SetContent(BuildOfficeCipher()); return; }
    if (bEntryMenu) { PanelHost->SetContent(MenuPage==EMenuPage::Pause ? BuildFrontEnd() : BuildMenuPage()); return; }
    if (bGiftReceipt) { PanelHost->SetContent(BuildGiftReceipt()); return; }
    if (!IsGameplayBlocked() && GetWorld()->GetSubsystem<UTripoStorySubsystem>()->HasDialogue())
    {
        PanelHost->SetContent(SNullWidget::NullWidget);
        StoryHost->SetContent(BuildStoryBubble());
        return;
    }
    if (!IsGameplayBlocked()) { PanelHost->SetContent(SNullWidget::NullWidget); return; }
    if (PlayerOwner && PlayerOwner->IsPaused() && !bEntryMenu && !bCollection && !bAbilityConfig) { PanelHost->SetContent(BuildMenuPage()); return; }
    const TWeakObjectPtr<ATripoHUD> WeakThis(this);
    TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
    auto Text = [&](FString Value) { Content->AddSlot().AutoHeight().Padding(0,6)[TripoMenu::Label(Value,18)]; };
    auto Heading = [&](const FString& Title,const FString& Subtitle=FString())
    { Content->AddSlot().AutoHeight()[TripoMenu::Heading(Title,Subtitle)]; };
    auto Button = [&](FString Title,FString Action)
    { Content->AddSlot().AutoHeight().Padding(0,5)[SNew(SButton).ButtonStyle(&MenuButtonStyle()).ButtonColorAndOpacity(FLinearColor(.08,.14,.11)).ContentPadding(FMargin(18,11)).OnClicked_Lambda([WeakThis,Action] { if (WeakThis.IsValid()) WeakThis->HandleAction(Action); return FReply::Handled(); })[TripoMenu::Label(Title,18,TripoMenu::Paper,true,false)]]; };
    auto* P = UTripoProgressSubsystem::Get(this); auto* Story = GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    if (bEntryMenu)
    {
        Heading(TEXT("明天寄来的礼物"),TEXT("一份礼物，一段还没走完的旅途。"));
        Button(TEXT("新游戏"),TEXT("new")); Button(TEXT("继续安全存档"),TEXT("load")); Button(TEXT("退出游戏"),TEXT("quit"));
    }
    else if (bCollection)
    {
        Heading(TEXT("收到的礼物"),TEXT("把一起走过的时光，好好收藏。"));
        Text(P->HasApplied(TEXT("Story.Home.GiftFound")) ? TEXT("童年玩具车 · 那块贴歪的地方也留着。") : TEXT("尚未找到第一件礼物"));
        Text(P->HasApplied(TEXT("Story.School.GiftFound")) ? TEXT("纸飞机 · 两个人的涂鸦。") : TEXT("尚未找到第二件礼物"));
        Text(P->HasApplied(TEXT("Story.Minecraft.PhotoFound")) ? TEXT("共同合影 · 曾经一起走过的地方。") : TEXT("尚未找到第三件礼物"));
        if (P->GetReplyChoice() >= 0) { static const TCHAR* Replies[] = {TEXT("收到啦"),TEXT("下次一起玩"),TEXT("我还想再试一次")}; Text(FString(TEXT("寄出的回信：")) + Replies[P->GetReplyChoice()]); }
        Button(TEXT("返回"),TEXT("collection.close"));
    }
    else if (bAbilityConfig && CanConfigureAbilities() && Player())
    {
        Heading(TEXT("能力配置"),TEXT("选择等级立即生效 · F2 返回游戏"));
        Button(Player()->bDebugFlying ? TEXT("关闭自由飞行 · 恢复正常移动") : TEXT("开启自由飞行测试"),TEXT("debug.fly"));
        Text(TEXT("飞行：WASD 移动 · 鼠标转向 · Space 上升 · Ctrl 下降 · Shift 加速；保留墙体碰撞。"));
        Content->AddSlot().AutoHeight().Padding(8)[SNew(STextBlock).ColorAndOpacity(FLinearColor(.025,.055,.04))
            .Font(TripoMenu::Font(14)).Text(FText::FromString(TEXT("0 级移除，1–3 级获得或升级。保存安全进度可保留配置，新游戏会重置。")))
            .AutoWrapText(true).WrapTextAt(700)];
        static const TCHAR* Controls[] = {TEXT("Shift · 水平冲刺"), TEXT("Ctrl · 向上移动"), TEXT("空中 Space · 蹬墙跳"), TEXT("Q · 放置垫脚石"), TEXT("F · 减慢目标"), TEXT("R 自身 / T 机关"), TEXT("C 创建 / 长按切换 · X 回收 / 长按选择"), TEXT("被动 · 挑战额外时间")};
        for (int32 I = 0; I < 8; ++I)
        {
            const int32 Current = Player()->Abilities->GetLevel(ETripoAbility(I));
            TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
            Row->AddSlot().FillWidth(1).VAlign(VAlign_Center)[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).ColorAndOpacity(FLinearColor(.025,.055,.04)).Font(TripoMenu::Font(18)).Text(FText::FromString(AbilityNames[I]))]
                + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).ColorAndOpacity(TripoMenu::Muted).Font(TripoMenu::Font(12)).AutoWrapText(true).Text(FText::FromString(Controls[I]))]];
            for (int32 Level = 0; Level <= 3; ++Level)
            {
                const FString Action = FString::Printf(TEXT("abilities.set.%d.%d"), I, Level);
                Row->AddSlot().AutoWidth().Padding(4,0)[SNew(SButton).ButtonStyle(&MenuButtonStyle()).ContentPadding(FMargin(12,8))
                    .ButtonColorAndOpacity(Current == Level ? FLinearColor(.25,.43,.3) : FLinearColor(.1,.16,.12))
                    .OnClicked_Lambda([WeakThis,Action] { if (WeakThis.IsValid()) WeakThis->HandleAction(Action); return FReply::Handled(); })
                    [SNew(STextBlock).ColorAndOpacity(TripoMenu::Paper).Font(TripoMenu::Font(14)).Text(FText::FromString(Level == 0 ? TEXT("移除") : FString::Printf(TEXT("%d 级"), Level)))]];
            }
            Content->AddSlot().AutoHeight().Padding(8,5)[Row];
        }
        Button(TEXT("全部获得 · 1 级"), TEXT("abilities.all.1"));
        Button(TEXT("全部升满 · 3 级"), TEXT("abilities.all.3"));
        Button(TEXT("移除全部能力"), TEXT("abilities.all.0"));
        Button(TEXT("返回菜单"), TEXT("abilities.close"));
        Button(TEXT("完成配置，继续游戏"), TEXT("resume"));
    }
    else if (PlayerOwner && PlayerOwner->IsPaused())
    {
        Text(TEXT("暂停\n读取游戏从最后安全存档开始，未结算的挑战会重开。"));
        Button(TEXT("继续当前游戏"),TEXT("resume")); Button(TEXT("保存安全进度"),TEXT("save"));
        if (CanConfigureAbilities()) Button(TEXT("能力配置 · 自由获得与调整"), TEXT("abilities.open"));
        Button(TEXT("读取安全存档"),TEXT("load")); Button(TEXT("新游戏"),TEXT("new")); Button(TEXT("退出游戏"),TEXT("quit"));
        Button(TEXT("查看纪念物与回信"),TEXT("collection"));
        Button(P->IsLabWorld(this) ? TEXT("返回故事存档") : TEXT("保存并进入独立练习场"),TEXT("practice"));
        if (P->IsLabWorld(this)) Button(TEXT("练习：解锁全部三级能力"),TEXT("practice.unlock"));
    }
    else if (Story->HasDialogue())
    {
        Content->AddSlot().AutoHeight().Padding(0,0,0,10)[TripoMenu::Label(Story->GetCurrent().Title.ToString(),24,TripoMenu::Brass,true)];
        Content->AddSlot().AutoHeight().Padding(0,0,0,12)[TripoMenu::Label(Story->GetCurrent().Text.ToString(),19)];
        auto Actions=SNew(SHorizontalBox);
        auto Choice=[&](const TCHAR* Label,const TCHAR* Action)
        {
            const FString Command(Action);
            Actions->AddSlot().AutoWidth().Padding(0,0,12,0)
                [SNew(SButton).ButtonStyle(&MenuButtonStyle()).ButtonColorAndOpacity(FLinearColor(.08,.14,.11)).ContentPadding(FMargin(12,9))
                    .OnClicked_Lambda([WeakThis,Command]{ if (WeakThis.IsValid()) WeakThis->HandleAction(Command); return FReply::Handled(); })
                    [SNew(STextBlock).ColorAndOpacity(FLinearColor(.95,.92,.82)).Font(TripoMenu::Font(16)).Text(FText::FromString(Label))]];
        };
        if (Story->GetCurrent().Id == TEXT("Story.Finale.SendReply") && P->GetReplyChoice() == INDEX_NONE)
        { Choice(TEXT("收到啦"),TEXT("reply.0")); Choice(TEXT("下次一起玩"),TEXT("reply.1")); Choice(TEXT("我还想再试一次"),TEXT("reply.2")); }
        else { Choice(TEXT("继续"),TEXT("story")); Choice(TEXT("跳过对话"),TEXT("skip")); }
        Content->AddSlot().AutoHeight().Padding(4)[Actions];
    }
    else if (bExchange)
    {
        Heading(TEXT("交换能力"),TEXT("先选减少的能力，再选增加的能力。受保护的基础等级不能交换。"));
        for (int32 I=0; I<8; ++I)
        {
            Button(FString::Printf(TEXT("%s 减少：%s（当前 %d）"),ExchangeFrom == I ? TEXT("●") : TEXT("○"),AbilityNames[I],Player()->Abilities->GetLevel(ETripoAbility(I))),FString::Printf(TEXT("from.%d"),I));
            Button(FString::Printf(TEXT("%s 增加：%s（当前 %d）"),ExchangeTo == I ? TEXT("●") : TEXT("○"),AbilityNames[I],Player()->Abilities->GetLevel(ETripoAbility(I))),FString::Printf(TEXT("to.%d"),I));
        }
        Button(TEXT("确认：来源 -1，目标 +1"),TEXT("exchange")); Button(TEXT("取消"),TEXT("cancel"));
    }
    else
    {
        Heading(TEXT("挑战完成"),P->IsChoiceEligible()?TEXT("选择一项能力，让下一段旅途多一种可能。"):TEXT("本次随机奖励已确定，领取后继续旅途。"));
        const auto Options = P->GetCandidates();
        if (Options.IsEmpty()) { Text(TEXT("没有可升级条目。本次记录完成，不会卡在奖励界面。")); Button(TEXT("确认完成"),TEXT("reward.0")); }
        else if (P->IsChoiceEligible()) for (int32 I=0; I<Options.Num(); ++I) Button(FString(AbilityNames[uint8(Options[I].Ability)]) + TEXT(" +1"),FString::Printf(TEXT("reward.%d"),I));
        else Button(TEXT("领取本次随机升级"),TEXT("reward.0"));
    }
    const FSlateBrush* Art=&PaperBrush;
    FVector2D Size(960,720); FMargin Inset(120,108,120,100);
    if (bCollection) { Art=&ItemBrush; Inset=FMargin(132,148,120,115); }
    else if (Story->HasDialogue()) { Art=&DialogueBrush; Size=FVector2D(1080,450); Inset=FMargin(145,119,150,95); }
    else if (bAbilityConfig || bExchange || P->GetPhase()==ETripoChallengePhase::PendingReward) Art=&UpgradeBrush;
    PanelHost->SetContent(FramePanel(SNew(SScrollBox).AnimateWheelScrolling(true).WheelScrollMultiplier(0.7f).ConsumeMouseWheel(EConsumeMouseWheel::Always)+SScrollBox::Slot()[Content],Art,Size,Inset));
}
void ATripoHUD::HandleAction(FString Action)
{
    if (bEntryMenu)
    {
        auto* Progress=UTripoProgressSubsystem::Get(this);
        if (Action==TEXT("new")) { bConfirmNewGame=true; PanelKey.Empty(); return; }
        if (Action==TEXT("front.cancel")) { bConfirmNewGame=false; PanelKey.Empty(); return; }
        if (Action==TEXT("front.start")) { Progress->ConsumeEntryMenu(); Progress->NewGame(false); return; }
        if (Action==TEXT("load")) { if(!Progress->ContinueGame(false)) FrontEndMessage=TEXT("未找到可用的安全存档，请开始新旅程。"); PanelKey.Empty(); return; }
        if (Action==TEXT("quit")) { UKismetSystemLibrary::QuitGame(this,PlayerOwner,EQuitPreference::Quit,false); return; }
    }
    if (Action==TEXT("front.return"))
    {
        UTripoRuntimeSubsystem::GetRuntime(this)->SetPauseReason(ETripoPauseReason::Menu,false);
        UGameplayStatics::OpenLevel(this,TEXT("/Game/Maps/L_MainMenu")); return;
    }
    if (bGiftReceipt)
    {
        if (ChapterGift.IsValid())
        {
            if (Action==TEXT("gift.choose.full") || Action.StartsWith(TEXT("gift.choose.")))
            {
                const FString Value=Action.RightChop(12);
                if ((Value==TEXT("full") || Value.IsNumeric()) && ChapterGift->ChooseSkill(Player(),Value==TEXT("full") ? INDEX_NONE : FCString::Atoi(*Value)))
                {
                    ChapterGift.Reset(); bGiftReceipt=false; PanelKey.Empty();
                    UTripoRuntimeSubsystem::GetRuntime(this)->SetPauseReason(ETripoPauseReason::Reward,false);
                }
            }
            return;
        }
        if (Action == TEXT("gift.confirm") && FPlatformTime::Seconds()-GiftRevealStart >= .85)
        {
            if (auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) R->SetPauseReason(ETripoPauseReason::Reward,false);
            bGiftReceipt=false; PanelKey.Empty();
        }
        return;
    }
    auto* C = Player(); auto* P = UTripoProgressSubsystem::Get(this); auto* Story = GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    if (Action.StartsWith(TEXT("ui.")))
    {
        if (!bEntryMenu && (!PlayerOwner || !PlayerOwner->IsPaused())) return;
        if (Action == TEXT("ui.settings")) MenuPage=EMenuPage::Settings;
        else if (Action == TEXT("ui.preferences")) MenuPage=EMenuPage::Preferences;
        else if (Action == TEXT("ui.more")) MenuPage=EMenuPage::More;
        else if (Action == TEXT("ui.tutorial")) MenuPage=EMenuPage::Tutorial;
        else if (Action == TEXT("ui.handbook")) MenuPage=EMenuPage::Handbook;
        else if (Action == TEXT("ui.back")) NavigateBack();
        else if (Action.StartsWith(TEXT("ui.skill.")))
        {
            int32 Index=INDEX_NONE;
            if (LexTryParseString(Index,*Action.Mid(9)) && Index>=0 && Index<8) HandbookAbility=Index;
        }
        PanelKey.Empty(); return;
    }
    if (!C) return;
    if (Action == TEXT("debug.fly") && bAbilityConfig && CanConfigureAbilities())
    {
        C->SetDebugFlying(!C->bDebugFlying);
        bAbilityConfig=false;
        UTripoRuntimeSubsystem::GetRuntime(this)->SetPauseReason(ETripoPauseReason::Menu,false);
        PanelKey.Empty(); return;
    }
    if (Action == TEXT("resume")) UTripoRuntimeSubsystem::GetRuntime(this)->SetPauseReason(ETripoPauseReason::Menu,false);
    else if (Action == TEXT("save")) P->SaveSafe(C);
    else if (Action == TEXT("load")) P->ContinueGame(P->IsLabWorld(this));
    else if (Action == TEXT("new")) P->NewGame(false);
    else if (Action == TEXT("abilities.toggle") && CanConfigureAbilities())
    {
        if (bAbilityConfig)
        {
            bAbilityConfig = false;
            UTripoRuntimeSubsystem::GetRuntime(this)->SetPauseReason(ETripoPauseReason::Menu, false);
        }
        else if (!bEntryMenu && !bCollection && !bExchange && !Story->HasDialogue() && P->GetPhase() != ETripoChallengePhase::PendingReward)
        {
            UTripoRuntimeSubsystem::GetRuntime(this)->SetPauseReason(ETripoPauseReason::Menu, true);
            bAbilityConfig = true;
        }
    }
    else if (Action == TEXT("abilities.open") && CanConfigureAbilities() && PlayerOwner && PlayerOwner->IsPaused()) bAbilityConfig = true;
    else if (Action == TEXT("abilities.close")) bAbilityConfig = false;
    else if (Action.StartsWith(TEXT("abilities.")) && bAbilityConfig && CanConfigureAbilities() && PlayerOwner && PlayerOwner->IsPaused())
    {
        TArray<FString> Parts;
        Action.ParseIntoArray(Parts, TEXT("."), false);
        TArray<int32> Levels = C->Abilities->ExportLevels();
        int32 Index = INDEX_NONE, Level = INDEX_NONE;
        bool bValid = false;
        if (Parts.Num() == 4 && Parts[1] == TEXT("set") && LexTryParseString(Index, *Parts[2]) && LexTryParseString(Level, *Parts[3]) && Levels.IsValidIndex(Index) && Level >= 0 && Level <= 3)
        { Levels[Index] = Level; bValid = true; }
        else if (Parts.Num() == 3 && Parts[1] == TEXT("all") && LexTryParseString(Level, *Parts[2]) && Level >= 0 && Level <= 3)
        { Levels.Init(Level, 8); bValid = true; }
        if (bValid)
        {
            C->Abilities->CancelAll();
            if (C->Abilities->ImportLevels(Levels))
            {
                TArray<double> Cooldowns; Cooldowns.Init(0., 8);
                C->Abilities->ImportCooldowns(Cooldowns);
            }
        }
    }
    else if (Action == TEXT("collection")) bCollection = true;
    else if (Action == TEXT("collection.close")) bCollection = false;
    else if (Action == TEXT("practice.unlock") && P->IsLabWorld(this))
    { for (uint8 I=0; I<8; ++I) C->Abilities->GrantLevelFloor(ETripoAbility(I),3); }
    else if (Action == TEXT("practice"))
    {
        if (P->IsLabWorld(this)) P->ContinueGame(false);
        else if (P->SaveSafe(C)) P->NewGame(true);
    }
    else if (Action == TEXT("quit")) UKismetSystemLibrary::QuitGame(this,PlayerOwner,EQuitPreference::Quit,false);
    else if (Action == TEXT("story")) Story->AdvanceDialogue();
    else if (Action == TEXT("skip")) Story->CloseEvent(true);
    else if (Action.StartsWith(TEXT("reply.")) && Story->HasDialogue() && Story->GetCurrent().Id == TEXT("Story.Finale.SendReply"))
    { if (P->SelectReply(FCString::Atoi(*Action.Mid(6)))) { Story->CloseEvent(false); P->SaveSafe(C); } }
    else if (Action == TEXT("cancel")) bExchange = false;
    else if (Action.StartsWith(TEXT("from."))) ExchangeFrom = FCString::Atoi(*Action.Mid(5));
    else if (Action.StartsWith(TEXT("to."))) ExchangeTo = FCString::Atoi(*Action.Mid(3));
    else if (Action == TEXT("exchange"))
    { TArray<int32> Floors; Floors.Init(0,8); if (P->Exchange(C,ExchangeTransaction,ETripoAbility(ExchangeFrom),ETripoAbility(ExchangeTo),Floors)) { bExchange=false; P->SaveSafe(C); } }
    else if (Action.StartsWith(TEXT("reward."))) P->CommitReward(C,FCString::Atoi(*Action.Mid(7)));
    PanelKey.Empty();
}
