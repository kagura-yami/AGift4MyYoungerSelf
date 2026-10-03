#include "Misc/AutomationTest.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Tests/TripoAbilityProbe.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoAbilityDataTest, "Tripothon.Abilities.DefinitionsAndGrants", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoAbilityDataTest::RunTest(const FString&)
{
    auto* C = NewObject<UTripoAbilityComponent>();
    TestTrue(TEXT("Initialize eight default definitions"), C->InitializeDefinitions());
    for (uint8 Id = 0; Id < 8; ++Id)
    {
        const auto Ability = static_cast<ETripoAbility>(Id);
        auto* D = UTripoAbilityDefinition::MakeDefaults(C, Ability);
        TestTrue(TEXT("Defaults valid"), D->IsValidDefinition());
        TestNull(TEXT("Level zero has no parameters"), D->Parameters(0));
        TestTrue(TEXT("Grant level two"), C->GrantLevelFloor(Ability, 2));
        TestTrue(TEXT("Repeated lower floor succeeds"), C->GrantLevelFloor(Ability, 1));
        TestEqual(TEXT("No stacking or downgrade"), C->GetLevel(Ability), 2);
        TestFalse(TEXT("Challenge floor only checks"), C->MeetsLevelFloor(Ability, 3));
        TestFalse(TEXT("Reject level four"), C->GrantLevelFloor(Ability, 4));
    }
    auto* StoneDefaults=UTripoAbilityDefinition::MakeDefaults(C,ETripoAbility::StepStone);
    for (int32 Level=1; Level<=3; ++Level)
    {
        TestEqual(TEXT("Stone has a short real placement debounce"),StoneDefaults->Parameters(Level)->Cooldown,.1f);
        TestEqual(TEXT("Stone retains per-level capacity"),StoneDefaults->Parameters(Level)->Capacity,Level);
    }
    C->InitializeDefinitions();
    TestEqual(TEXT("Reinitialize retains acquired level"), C->GetLevel(ETripoAbility::Dash), 2);
    TestEqual(TEXT("Bonus level two is ten seconds"), C->GetBonusTimeSeconds(), 10.f);
    FGuid Handle;
    TestEqual(TEXT("Missing target fails before executor"), C->TryActivate(ETripoAbility::Slow, nullptr, Handle), ETripoAbilityFailure::NoTarget);
    TestFalse(TEXT("Failure creates no handle"), Handle.IsValid());
    TestEqual(TEXT("Failure creates no cooldown"), C->GetCooldownRemaining(ETripoAbility::Slow), 0.);
    TestEqual(TEXT("Bonus cannot be actively cast"), C->TryActivate(ETripoAbility::BonusTime, nullptr, Handle), ETripoAbilityFailure::Passive);
    auto* Invalid = NewObject<UTripoAbilityComponent>();
    auto* D = UTripoAbilityDefinition::MakeDefaults(Invalid, ETripoAbility::Dash);
    D->Levels[0].Cooldown = -1;
    Invalid->Definitions.Add(D);
    TestFalse(TEXT("Invalid catalog rejected atomically"), Invalid->InitializeDefinitions());
    TestFalse(TEXT("Failed initialization cannot grant"), Invalid->GrantLevelFloor(ETripoAbility::Dash, 1));
    D->Levels[0].Cooldown = 1;
    Invalid->Definitions.Add(D);
    TestFalse(TEXT("Duplicate definitions rejected"), Invalid->InitializeDefinitions());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoAbilityLifecycleTest, "Tripothon.Abilities.Lifecycle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoAbilityLifecycleTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    auto* GI = NewObject<UGameInstance>();
    World->SetGameInstance(GI);
    GI->Init();
    auto* Actor = World->SpawnActor<AActor>();
    auto* C = NewObject<UTripoAbilityComponent>(Actor);
    Actor->AddInstanceComponent(C);
    C->RegisterComponent();
    auto* D = UTripoAbilityDefinition::MakeDefaults(C, ETripoAbility::Dash);
    D->Implementation = UTripoAbilityProbe::StaticClass();
    D->Levels[0].Duration = 100;
    D->Levels[0].Cooldown = 200;
    C->Definitions.Add(D);
    auto* Short = UTripoAbilityDefinition::MakeDefaults(C, ETripoAbility::UpDash);
    Short->Implementation = UTripoAbilityProbe::StaticClass();
    Short->Levels[0].Duration = 1.e-9f;
    Short->Levels[0].Cooldown = 0;
    C->Definitions.Add(Short);
    auto* Stone = UTripoAbilityDefinition::MakeDefaults(C, ETripoAbility::StepStone);
    Stone->Implementation = UTripoAbilityProbe::StaticClass();
    Stone->Levels[0].Duration = 0;
    C->Definitions.Add(Stone);
    auto* Disabled = UTripoAbilityDefinition::MakeDefaults(C, ETripoAbility::Echo);
    Disabled->Implementation = nullptr;
    C->Definitions.Add(Disabled);
    C->InitializeDefinitions();
    C->GrantLevelFloor(ETripoAbility::Dash, 1);
    FGuid Handle;
    TestEqual(TEXT("Valid activation"), C->TryActivate(ETripoAbility::Dash, nullptr, Handle), ETripoAbilityFailure::None);
        TestTrue(TEXT("Unique active handle"), Handle.IsValid());
    UTripoAbilityProbe* Probe = nullptr;
    for (TObjectIterator<UTripoAbilityProbe> It; It; ++It) if (It->GetOuter() == C && It->GetHandle() == Handle) Probe = *It;
    if (TestNotNull(TEXT("Component owns executable UObject"), Probe))
    {
        C->TickComponent(.016f, LEVELTICK_All, nullptr);
        TestTrue(TEXT("Component advances UObject with action time"), Probe->UpdatedSeconds > 0);
        FGuid Rejected;
        TestEqual(TEXT("Duplicate activation rejected"), C->TryActivate(ETripoAbility::Dash, nullptr, Rejected), ETripoAbilityFailure::AlreadyActive);
        TestTrue(TEXT("Cancel by matching handle"), C->Cancel(Handle));
        TestFalse(TEXT("Stale cancellation is harmless"), C->Cancel(Handle));
        TestEqual(TEXT("Cleanup once"), Probe->Ends, 1);
        TestTrue(TEXT("Cleanup knows cancellation"), Probe->bWasCancelled);
        TestTrue(TEXT("Cancel does not refund cooldown"), C->GetCooldownRemaining(ETripoAbility::Dash) > 199);
        TestEqual(TEXT("Cooldown blocks recast"), C->TryActivate(ETripoAbility::Dash, nullptr, Rejected), ETripoAbilityFailure::Cooldown);
    }
    C->GrantLevelFloor(ETripoAbility::Echo, 1);
    TestEqual(TEXT("Unimplemented ability never fakes success"), C->TryActivate(ETripoAbility::Echo, nullptr, Handle), ETripoAbilityFailure::NotImplemented);
    C->GrantLevelFloor(ETripoAbility::StepStone,1);
    TestEqual(TEXT("Stone placement starts"),C->TryActivate(ETripoAbility::StepStone,nullptr,Handle),ETripoAbilityFailure::None);
    TestTrue(TEXT("Stone has real remaining cooldown"),C->GetCooldownRemaining(ETripoAbility::StepStone)>0);
    TestEqual(TEXT("Immediate repeated placement is blocked"),C->TryActivate(ETripoAbility::StepStone,nullptr,Handle),ETripoAbilityFailure::Cooldown);
    C->GrantLevelFloor(ETripoAbility::UpDash, 1);
    TestEqual(TEXT("Short effect starts"), C->TryActivate(ETripoAbility::UpDash, nullptr, Handle), ETripoAbilityFailure::None);
    UTripoAbilityProbe* ShortProbe = nullptr;
    for (TObjectIterator<UTripoAbilityProbe> It; It; ++It) if (It->GetOuter() == C && It->GetHandle() == Handle) ShortProbe = *It;
    if (TestNotNull(TEXT("Second owned executor"), ShortProbe))
    {
        TestTrue(TEXT("Active hit accepted"), C->ReportHit(Handle, FHitResult()));
        C->TickComponent(.016f, LEVELTICK_All, nullptr);
        TestFalse(TEXT("Duration expires"), ShortProbe->IsActive());
        TestEqual(TEXT("Natural cleanup exactly once"), ShortProbe->Ends, 1);
        TestFalse(TEXT("Expiry is not cancellation"), ShortProbe->bWasCancelled);
        TestFalse(TEXT("Old hit cannot reach next activation"), C->ReportHit(Handle, FHitResult()));
        const FGuid OldHandle = Handle;
        C->TryActivate(ETripoAbility::UpDash, nullptr, Handle);
        TestTrue(TEXT("Every activation has fresh ID"), OldHandle != Handle);
        auto* Runtime = UTripoRuntimeSubsystem::GetRuntime(C);
        Runtime->AdvanceRestore(ETripoRestorePhase::Locked);
        C->TickComponent(.016f, LEVELTICK_All, nullptr);
        TestTrue(TEXT("Restore lock cancels active effects"), ShortProbe->bWasCancelled);
        TestEqual(TEXT("Restore cleanup once"), ShortProbe->Ends, 2);
        TestEqual(TEXT("No activation during restore"), C->TryActivate(ETripoAbility::UpDash, nullptr, Handle), ETripoAbilityFailure::Restoring);
        C->CancelAll();
        TestEqual(TEXT("Bulk cleanup idempotent"), ShortProbe->Ends, 2);
    }
    GI->Shutdown();
    World->DestroyWorld(false);
    return true;
}
#endif
