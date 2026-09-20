#include "Misc/AutomationTest.h"
#include "Core/TripoRuntimeState.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoIdentityRegistry.h"
#include "UObject/UnrealType.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoClockTest, "Tripothon.Runtime.ActionClock", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoClockTest::RunTest(const FString&)
{
    FTripoActionClock Clock;
    Clock.Sample(100.);
    Clock.SetPaused(ETripoPauseReason::Menu, true, 102.);
    Clock.SetPaused(ETripoPauseReason::Loading, true, 105.);
    Clock.SetPaused(ETripoPauseReason::Menu, false, 110.);
    Clock.Sample(120.);
    TestEqual(TEXT("Closing menu cannot cancel loading pause"), Clock.Seconds, 2.);
    Clock.SetPaused(ETripoPauseReason::Loading, false, 130.);
    Clock.Sample(131.);
    TestEqual(TEXT("Resume does not catch up paused interval"), Clock.Seconds, 3.);
    Clock.Sample(129.);
    Clock.Sample(132.);
    TestEqual(TEXT("Backward samples neither rewind nor double count"), Clock.Seconds, 4.);
    Clock.SetPaused(ETripoPauseReason::Menu, true, 132.);
    Clock.SetPaused(ETripoPauseReason::Menu, true, 135.);
    Clock.SetPaused(ETripoPauseReason::Menu, false, 140.);
    Clock.Sample(141.);
    TestEqual(TEXT("Reasons are idempotent"), Clock.Seconds, 5.);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoRestoreTest, "Tripothon.Runtime.RestoreOrder", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoRestoreTest::RunTest(const FString&)
{
    FTripoRestoreState State;
    TestFalse(TEXT("Cannot skip locking"), State.Advance(ETripoRestorePhase::PlayerRestored));
    TestEqual(TEXT("Rejected transition preserves epoch"), State.Epoch, int64(0));
    for (int32 Phase = 1; Phase <= 6; ++Phase)
        TestTrue(TEXT("Ordered recovery step"), State.Advance(static_cast<ETripoRestorePhase>(Phase)));
    TestEqual(TEXT("History barrier increments epoch once"), State.Epoch, int64(1));
    TestFalse(TEXT("Cannot repeat history clear"), State.Advance(ETripoRestorePhase::HistoryCleared));
    TestTrue(TEXT("Unlock only after overlap refresh"), State.Advance(ETripoRestorePhase::Running));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoDuplicateIdentityTest, "Tripothon.Runtime.DuplicateIdentity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoDuplicateIdentityTest::RunTest(const FString&)
{
    auto* Registry = NewObject<UTripoIdentityRegistry>();
    auto* First = NewObject<UTripoIdentityComponent>();
    auto* Second = NewObject<UTripoIdentityComponent>();
    // Simulate two serialized components carrying the same corrupted asset ID.
    auto* Property = FindFProperty<FStructProperty>(UTripoIdentityComponent::StaticClass(), TEXT("StableId"));
    if (!TestNotNull(TEXT("Serialized stable ID property exists"), Property)) return false;
    const FGuid Id = FGuid::NewGuid();
    *Property->ContainerPtrToValuePtr<FGuid>(First) = Id;
    *Property->ContainerPtrToValuePtr<FGuid>(Second) = Id;
    TestTrue(TEXT("First registration accepted"), Registry->Register(First));
    TestTrue(TEXT("Same component registration is idempotent"), Registry->Register(First));
    AddExpectedError(TEXT("Duplicate stable ID"), EAutomationExpectedErrorFlags::Contains, 1);
    TestFalse(TEXT("Conflicting component rejected"), Registry->Register(Second));
    TestNull(TEXT("Ambiguous identity cannot resolve to arbitrary actor"), Registry->Resolve(Id));
    Registry->Unregister(Second);
    TestFalse(TEXT("Removing duplicate cannot silently unquarantine identity"), Registry->Register(First));
    return true;
}
#endif
