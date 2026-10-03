#include "Misc/AutomationTest.h"
#include "Progress/TripoSaveGame.h"
#include "Progress/TripoRewardRules.h"
#include "Kismet/GameplayStatics.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoGiftSaveTest,"Tripothon.Gift.ReceiptPersistence",EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoGiftSaveTest::RunTest(const FString&)
{
    auto* Save=NewObject<UTripoSaveGame>();
    Save->RunId=FGuid::NewGuid(); Save->Levels.Init(0,8); Save->MapPackage=TEXT("/Game/Maps/L_ChaseWhitebox");
    TestTrue(TEXT("Older schema without gift receipts remains valid"),Save->IsValidData());
    FTripoGiftReceipt Reward; Reward.AbilityIndex=3; Reward.GrantedLevel=2;
    Save->Gifts.Add(TEXT("/Game/Maps/L_ChaseWhitebox:Gift:A"),Reward);
    Save->Gifts.Add(TEXT("/Game/Maps/L_ChaseWhitebox:Gift:Empty"),FTripoGiftReceipt());
    Save->Levels[3]=2;
    TArray<uint8> Data;
    TestTrue(TEXT("Serialize save without touching player slots"),UGameplayStatics::SaveGameToMemory(Save,Data));
    auto* Loaded=Cast<UTripoSaveGame>(UGameplayStatics::LoadGameFromMemory(Data));
    if (!TestNotNull(TEXT("Deserialize save"),Loaded)) return false;
    TestTrue(TEXT("Roundtrip is valid"),Loaded->IsValidData());
    TestEqual(TEXT("Gift ledger preserved"),Loaded->Gifts.Num(),2);
    TestEqual(TEXT("Ability level and receipt travel together"),Loaded->Levels[3],2);
    const auto* Receipt=Loaded->Gifts.Find(TEXT("/Game/Maps/L_ChaseWhitebox:Gift:A"));
    if (TestNotNull(TEXT("Stable receipt key preserved"),Receipt)) TestEqual(TEXT("Granted level preserved"),Receipt->GrantedLevel,2);
    Loaded->Gifts.FindChecked(TEXT("/Game/Maps/L_ChaseWhitebox:Gift:A")).AbilityIndex=8;
    TestFalse(TEXT("Reject invalid ability in gift ledger"),Loaded->IsValidData());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoGiftPoolTest,"Tripothon.Gift.WeightedPool",EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoGiftPoolTest::RunTest(const FString&)
{
    TArray<int32> Levels; Levels.Init(3,8); Levels[3]=0;
    TArray<FTripoRewardOption> Pool;
    for (uint8 I=0; I<8; ++I) { FTripoRewardOption O; O.Ability=ETripoAbility(I); O.Weight=I+1; Pool.Add(O); }
    auto Eligible=TripoReward::Filter(Pool,Levels);
    TestEqual(TEXT("Only non-maxed ability remains"),Eligible.Num(),1);
    TestEqual(TEXT("A locked ability can be unlocked"),int32(Eligible[0].Ability),3);
    TestEqual(TEXT("One-item pool deterministically selects it"),TripoReward::Draw(Eligible,42,TEXT("Box.A")),0);
    Levels[3]=3;
    TestTrue(TEXT("All maxed produces keepsake fallback"),TripoReward::Filter(Pool,Levels).IsEmpty());
    TestEqual(TEXT("Empty pool has no invalid index"),TripoReward::Draw({},42,TEXT("Box.Empty")),INDEX_NONE);
    return true;
}
#endif
