#include "Lab/TripoHUD.h"
#include "Player/TripoCharacter.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Story/TripoStorySubsystem.h"
#include "World/TripoZone.h"
#include "World/TripoWorldSubsystem.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SNullWidget.h"
#include "Styling/CoreStyle.h"

static const TCHAR* AbilityNames[] = {TEXT("平面位移"),TEXT("上位移"),TEXT("蹬墙跳"),TEXT("垫脚石"),TEXT("局部减慢"),TEXT("时回"),TEXT("分身"),TEXT("奖励加时")};
ATripoCharacter* ATripoHUD::Player() const { return Cast<ATripoCharacter>(GetOwningPawn()); }
void ATripoHUD::BeginPlay()
{
    Super::BeginPlay(); const TWeakObjectPtr<ATripoHUD> WeakThis(this);
    auto* Progress = UTripoProgressSubsystem::Get(this);
    const bool bSkipEntryMenu = GIsEditor && bSkipEntryMenuInEditor;
    bEntryMenu = !bSkipEntryMenu && !Progress->IsLabWorld(this) && Progress->ConsumeEntryMenu();
    RootWidget = SNew(SOverlay)
    + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(18)
    [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.015,.025,.045,.85))
        [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 18)).Text_Lambda([WeakThis] { return WeakThis.IsValid() ? WeakThis->StatusText() : FText::GetEmpty(); }).AutoWrapText(true).WrapTextAt(760)]]
    + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(24)
    [SAssignNew(PanelHost, SBox).WidthOverride(760).MaxDesiredHeight(600)];
    if (GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->AddViewportWidgetContent(RootWidget.ToSharedRef(),10);
}
void ATripoHUD::EndPlay(const EEndPlayReason::Type Reason)
{
    if (RootWidget.IsValid() && GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(RootWidget.ToSharedRef());
    RootWidget.Reset(); PanelHost.Reset(); Super::EndPlay(Reason);
}
bool ATripoHUD::IsGameplayBlocked() const
{
    auto* P = UTripoProgressSubsystem::Get(this); auto* Story = GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    return bEntryMenu || bCollection || bExchange || (PlayerOwner && PlayerOwner->IsPaused()) || (Story && Story->HasDialogue()) || (P && P->GetPhase() == ETripoChallengePhase::PendingReward);
}
FText ATripoHUD::StatusText() const
{
    const auto* C = Player(); if (!C) return FText::GetEmpty(); auto* P = UTripoProgressSubsystem::Get(this);
    if (IsGameplayBlocked()) return FText::FromString(P->GetMessage());
    FString Text = TEXT("明天寄来的礼物\nWASD 移动  鼠标转向  Space 跳跃  E 交互  Esc 菜单\n");
    static const TCHAR* Controls[] = {TEXT("Shift"),TEXT("Ctrl"),TEXT("空中 Space"),TEXT("按住 Q 预览 / 松开放置"),TEXT("F"),TEXT("R 自身 / T 机关"),TEXT("C 生成 / 取消"),TEXT("被动")};
    for (uint8 I=0; I<8; ++I)
    {
        if (C->Abilities->GetLevel(ETripoAbility(I)) == 0) continue;
        const double Cooldown = C->Abilities->GetCooldownRemaining(ETripoAbility(I));
        Text += FString::Printf(TEXT("%s · %s Lv.%d%s\n"), Controls[I], AbilityNames[I], C->Abilities->GetLevel(ETripoAbility(I)), Cooldown > 0 ? *FString::Printf(TEXT(" [%.1fs]"),Cooldown) : TEXT(""));
    }
    if (P->GetPhase() == ETripoChallengePhase::Running)
        Text += FString::Printf(TEXT("\n挑战 %.2f / %.2f 秒 · %s\nBackspace 重开挑战"),P->GetElapsed(),P->GetBudget(),P->GetElapsed() <= P->GetBudget() ? TEXT("限时内可自选") : TEXT("已超时，终点随机；仍可继续"));
    if (!P->GetMessage().IsEmpty()) Text += TEXT("\n") + P->GetMessage();
    if (!C->Abilities->GetFailureMessage().IsEmpty()) Text += TEXT("\n") + C->Abilities->GetFailureMessage();
    if (!C->GetStonePreviewHint().IsEmpty()) Text += TEXT("\n") + C->GetStonePreviewHint();
    if (!UTripoWorldSubsystem::Get(this)->GetLastFailure().IsEmpty()) Text += TEXT("\n") + UTripoWorldSubsystem::Get(this)->GetLastFailure();
    return FText::FromString(Text);
}
void ATripoHUD::RefreshUI()
{
    if (!PanelHost.IsValid()) return;
    auto* P = UTripoProgressSubsystem::Get(this); auto* Story = GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    const FString Key = FString::Printf(TEXT("%d:%d:%d:%d:%d:%s"),PlayerOwner && PlayerOwner->IsPaused(),int32(P->GetPhase()),bExchange,ExchangeFrom,ExchangeTo,Story->HasDialogue() ? *Story->GetCurrent().Id.ToString() : TEXT(""));
    if (Key != PanelKey) { PanelKey = Key; RebuildPanel(); }
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
    if (!IsGameplayBlocked()) { PanelHost->SetContent(SNullWidget::NullWidget); return; }
    const TWeakObjectPtr<ATripoHUD> WeakThis(this);
    TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
    auto Text = [&](FString Value) { Content->AddSlot().AutoHeight().Padding(8)[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 22)).Text(FText::FromString(Value)).AutoWrapText(true).WrapTextAt(700)]; };
    auto Button = [&](FString Title,FString Action)
    { Content->AddSlot().AutoHeight().Padding(6)[SNew(SButton).ContentPadding(10).OnClicked_Lambda([WeakThis,Action] { if (WeakThis.IsValid()) WeakThis->HandleAction(Action); return FReply::Handled(); })[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular", 20)).Text(FText::FromString(Title))]]; };
    auto* P = UTripoProgressSubsystem::Get(this); auto* Story = GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    if (bEntryMenu)
    {
        Text(TEXT("明天寄来的礼物"));
        Button(TEXT("新游戏"),TEXT("new")); Button(TEXT("继续安全存档"),TEXT("load")); Button(TEXT("退出游戏"),TEXT("quit"));
    }
    else if (bCollection)
    {
        Text(TEXT("收到的礼物"));
        Text(P->HasApplied(TEXT("Story.Home.GiftFound")) ? TEXT("童年玩具车 · 那块贴歪的地方也留着。") : TEXT("尚未找到第一件礼物"));
        Text(P->HasApplied(TEXT("Story.School.GiftFound")) ? TEXT("纸飞机 · 两个人的涂鸦。") : TEXT("尚未找到第二件礼物"));
        Text(P->HasApplied(TEXT("Story.Minecraft.PhotoFound")) ? TEXT("共同合影 · 曾经一起走过的地方。") : TEXT("尚未找到第三件礼物"));
        if (P->GetReplyChoice() >= 0) { static const TCHAR* Replies[] = {TEXT("收到啦"),TEXT("下次一起玩"),TEXT("我还想再试一次")}; Text(FString(TEXT("寄出的回信：")) + Replies[P->GetReplyChoice()]); }
        Button(TEXT("返回"),TEXT("collection.close"));
    }
    else if (PlayerOwner && PlayerOwner->IsPaused())
    {
        Text(TEXT("暂停\n读取游戏从最后安全存档开始，未结算的挑战会重开。"));
        Button(TEXT("继续当前游戏"),TEXT("resume")); Button(TEXT("保存安全进度"),TEXT("save"));
        Button(TEXT("读取安全存档"),TEXT("load")); Button(TEXT("新游戏"),TEXT("new")); Button(TEXT("退出游戏"),TEXT("quit"));
        Button(TEXT("查看纪念物与回信"),TEXT("collection"));
        Button(P->IsLabWorld(this) ? TEXT("返回故事存档") : TEXT("保存并进入独立练习场"),TEXT("practice"));
        if (P->IsLabWorld(this)) Button(TEXT("练习：解锁全部三级能力"),TEXT("practice.unlock"));
    }
    else if (Story->HasDialogue())
    {
        Text(Story->GetCurrent().Title.ToString()); Text(Story->GetCurrent().Text.ToString());
        if (Story->GetCurrent().Id == TEXT("Story.Finale.SendReply") && P->GetReplyChoice() == INDEX_NONE)
        { Button(TEXT("收到啦 · 寄给明天"),TEXT("reply.0")); Button(TEXT("下次一起玩 · 寄给明天"),TEXT("reply.1")); Button(TEXT("我还想再试一次 · 寄给明天"),TEXT("reply.2")); }
        else { Button(TEXT("继续"),TEXT("story")); Button(TEXT("跳过表现（仍保留必要结果）"),TEXT("skip")); }
    }
    else if (bExchange)
    {
        Text(TEXT("纸盒邮差 · 一等级换一等级\n先选减少的能力，再选增加的能力。受保护的基础等级不能交换。取消不会改变等级。"));
        for (int32 I=0; I<8; ++I)
        {
            Button(FString::Printf(TEXT("%s 减少：%s（当前 %d）"),ExchangeFrom == I ? TEXT("●") : TEXT("○"),AbilityNames[I],Player()->Abilities->GetLevel(ETripoAbility(I))),FString::Printf(TEXT("from.%d"),I));
            Button(FString::Printf(TEXT("%s 增加：%s（当前 %d）"),ExchangeTo == I ? TEXT("●") : TEXT("○"),AbilityNames[I],Player()->Abilities->GetLevel(ETripoAbility(I))),FString::Printf(TEXT("to.%d"),I));
        }
        Button(TEXT("确认：来源 -1，目标 +1"),TEXT("exchange")); Button(TEXT("取消"),TEXT("cancel"));
    }
    else
    {
        Text(P->IsChoiceEligible() ? TEXT("挑战完成 · 选择一项升级") : TEXT("挑战完成 · 超时随机奖励已固定"));
        const auto Options = P->GetCandidates();
        if (Options.IsEmpty()) { Text(TEXT("没有可升级条目。本次记录完成，不会卡在奖励界面。")); Button(TEXT("确认完成"),TEXT("reward.0")); }
        else if (P->IsChoiceEligible()) for (int32 I=0; I<Options.Num(); ++I) Button(FString(AbilityNames[uint8(Options[I].Ability)]) + TEXT(" +1"),FString::Printf(TEXT("reward.%d"),I));
        else Button(TEXT("领取本次随机升级"),TEXT("reward.0"));
    }
    PanelHost->SetContent(SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).Padding(18).BorderBackgroundColor(FLinearColor(.025,.035,.06,.98))[SNew(SScrollBox) + SScrollBox::Slot()[Content]]);
}
void ATripoHUD::HandleAction(FString Action)
{
    auto* C = Player(); if (!C) return; auto* P = UTripoProgressSubsystem::Get(this); auto* Story = GetWorld()->GetSubsystem<UTripoStorySubsystem>();
    if (Action == TEXT("resume")) UTripoRuntimeSubsystem::GetRuntime(this)->SetPauseReason(ETripoPauseReason::Menu,false);
    else if (Action == TEXT("save")) P->SaveSafe(C);
    else if (Action == TEXT("load")) P->ContinueGame(P->IsLabWorld(this));
    else if (Action == TEXT("new")) P->NewGame(P->IsLabWorld(this));
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
    else if (Action == TEXT("story")) Story->CloseEvent(false);
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
