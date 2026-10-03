#include "Progress/TripoProgressSubsystem.h"
#include "Progress/TripoRewardRules.h"
#include "Player/TripoCharacter.h"
#include "Player/TripoPlayerController.h"
#include "Abilities/TripoAbilityComponent.h"
#include "World/TripoInteractorComponent.h"
#include "Core/TripoRuntimeSubsystem.h"

bool UTripoProgressSubsystem::ClaimGift(ATripoCharacter* Player,FName Key,const TArray<FTripoRewardOption>& Pool,FTripoGiftReceipt& OutReceipt)
{
    auto* R = UTripoRuntimeSubsystem::GetRuntime(Player);
    if (!IsValid(Player) || !Player->HasAuthority() || !Player->IsPlayerControlled() || Player->Interactor->bSuppressed ||
        !R || R->IsActionPaused() || R->GetRestorePhase()!=ETripoRestorePhase::Running || Key.IsNone() || Gifts.Contains(Key) || Gifts.Num()>=4096 || Phase==ETripoChallengePhase::PendingReward) return false;
    // Reject malformed designer data rather than silently consuming a broken box.
    TSet<ETripoAbility> Seen;
    for (const auto& Option : Pool)
    {
        if (uint8(Option.Ability)>=8 || !FMath::IsFinite(Option.Weight) || Option.Weight<=0 || Seen.Contains(Option.Ability))
        { Message=TEXT("礼物奖池配置有误，尚未领取"); return false; }
        Seen.Add(Option.Ability);
    }
    auto Levels = Player->Abilities->ExportLevels();
    TArray<ATripoCharacter*> Bodies = {Player};
    if (auto* PC=Cast<ATripoPlayerController>(Player->GetController())) Bodies=PC->GetEchoBodies();
    for (const auto* Body : Bodies)
        if (IsValid(Body)) for (uint8 I=0; I<8; ++I) Levels[I]=FMath::Max(Levels[I],Body->Abilities->GetLevel(ETripoAbility(I)));
    const auto GiftCandidates = TripoReward::Filter(Pool,Levels);
    const int32 Pick = TripoReward::Draw(GiftCandidates,RunSeed,Key);
    FTripoGiftReceipt Receipt;
    if (GiftCandidates.IsValidIndex(Pick))
    {
        const ETripoAbility Ability=GiftCandidates[Pick].Ability;
        Receipt.AbilityIndex=uint8(Ability);
        Receipt.GrantedLevel=Levels[Receipt.AbilityIndex]+1;
        if (!Player->Abilities->GrantLevelFloor(Ability,Receipt.GrantedLevel)) return false;
        for (auto* Body : Bodies) if (IsValid(Body) && Body!=Player) Body->Abilities->GrantLevelFloor(Ability,Receipt.GrantedLevel);
    }
    Gifts.Add(Key,Receipt);
    OutReceipt=Receipt;
    static const TCHAR* Names[]={TEXT("平面位移"),TEXT("上位移"),TEXT("蹬墙跳"),TEXT("垫脚石"),TEXT("局部减慢"),TEXT("时回"),TEXT("分身"),TEXT("奖励加时")};
    Message = Receipt.AbilityIndex==INDEX_NONE ? TEXT("收到纪念礼物：奖池中没有可升级的能力") :
        FString::Printf(TEXT("收到礼物：%s → Lv.%d"),Names[Receipt.AbilityIndex],Receipt.GrantedLevel);
    return true;
}
