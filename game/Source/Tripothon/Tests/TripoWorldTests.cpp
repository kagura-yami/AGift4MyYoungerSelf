#include "Misc/AutomationTest.h"
#include "World/TripoMechanism.h"
#include "World/TripoInteractorComponent.h"
#include "Progress/TripoRewardRules.h"
#include "Engine/World.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoMechanismTest, "Tripothon.World.MechanismGraph", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoMechanismTest::RunTest(const FString&)
{
    auto* World = UWorld::CreateWorld(EWorldType::Game, false);
    auto* A = World->SpawnActor<ATripoMechanism>(); A->Kind = ETripoMechanismKind::Switch;
    auto* B = World->SpawnActor<ATripoMechanism>(); B->Kind = ETripoMechanismKind::Switch;
    auto* Gate = World->SpawnActor<ATripoMechanism>(); Gate->Kind = ETripoMechanismKind::Gate; Gate->Inputs = {A, B};
    auto State = A->Capture(); State.bLatched = true; A->Restore(State);
    TestFalse(TEXT("AND requires both"), Gate->IsPowered());
    Gate->bRequireAll = false; TestTrue(TEXT("OR accepts one"), Gate->IsPowered());
    State.bLatched = false; A->Restore(State); A->Restore(State);
    TestFalse(TEXT("Repeated restore never toggles"), Gate->IsPowered());
    Gate->Inputs = {Gate}; TestFalse(TEXT("Cycle fails closed"), Gate->IsPowered());
    auto* I = NewObject<UTripoInteractorComponent>();
    I->Kind = ETripoInteractor::Memory; TestFalse(TEXT("Memory has no gameplay permissions"), I->CanTrigger(true));
    I->Kind = ETripoInteractor::Echo; TestFalse(TEXT("Echo requires opt-in"), I->CanTrigger(false));
    TestTrue(TEXT("Allowed echo can trigger"), I->CanTrigger(true));
    I->bSuppressed = true; TestFalse(TEXT("Replay suppression dominates permission"), I->CanTrigger(true));
    World->DestroyWorld(false); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoRewardTest, "Tripothon.Progress.RewardRules", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoRewardTest::RunTest(const FString&)
{
    TestTrue(TEXT("Exact boundary remains choice"), TripoReward::ChoiceEligible(30, 30));
    TestFalse(TEXT("Sub-frame excess is random"), TripoReward::ChoiceEligible(30.000001, 30));
    TArray<int32> Levels; Levels.Init(0, 8); Levels[0] = 3;
    TArray<FTripoRewardOption> Pool;
    for (uint8 I = 0; I < 8; ++I) { FTripoRewardOption R; R.Ability = ETripoAbility(I); R.Weight = I+1; Pool.Add(R); }
    auto Filtered = TripoReward::Filter(Pool, Levels);
    TestEqual(TEXT("Max-level excluded"), Filtered.Num(), 7);
    TestEqual(TEXT("Stable sorted IDs"), uint8(Filtered[0].Ability), uint8(1));
    const int32 First = TripoReward::Draw(Filtered, 12345, TEXT("TestChallenge"));
    for (int32 I = 0; I < 10; ++I) TestEqual(TEXT("Same seed does not reroll"), TripoReward::Draw(Filtered, 12345, TEXT("TestChallenge")), First);
    Levels.Init(3, 8); TestTrue(TEXT("All max-level returns empty fallback"), TripoReward::Filter(Pool, Levels).IsEmpty());
    TestEqual(TEXT("Empty pool cannot index"), TripoReward::Draw({}, 0, TEXT("Empty")), INDEX_NONE);
    auto* D = NewObject<UTripoChallengeDefinition>(); D->ChallengeId = TEXT("Valid"); D->Rewards = Pool;
    TestTrue(TEXT("Definition valid"), D->IsValidDefinition()); D->Rewards[0].Weight = -1;
    TestFalse(TEXT("Reject invalid weight"), D->IsValidDefinition());
    return true;
}
#endif
