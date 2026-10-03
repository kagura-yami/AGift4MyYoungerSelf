#include "Misc/AutomationTest.h"
#include "Player/TripoCharacter.h"
#include "Player/TripoMovementComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoJumpMomentumTest, "Tripothon.Movement.UpBurstMomentum", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoJumpMomentumTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    auto* GI = NewObject<UGameInstance>();
    World->SetGameInstance(GI);
    GI->Init();
    auto* Player = World->SpawnActor<ATripoCharacter>();
    auto* M = CastChecked<UTripoMovementComponent>(Player->GetCharacterMovement());
    const float JumpZ = FMath::Sqrt(M->UpDashHeightScale * (400.f * 400.f + 2.f * -M->GetGravityZ() * .2f * 400.f));
    for (float InitialZ : {300.f, -500.f})
    {
        M->SetMovementMode(MOVE_Falling);
        M->ResetAirUses();
        M->Velocity = FVector(300, 200, InitialZ);
        TestTrue(TEXT("Ctrl starts while rising or falling"), M->BeginBurst(true, FVector::ForwardVector, 400, .2f));
        TestTrue(TEXT("Upward velocity replaces previous vertical motion immediately"), M->Velocity.Equals(FVector(300, 200, JumpZ)));
        TestFalse(TEXT("Second jump never enters constant-speed custom movement"), M->IsBursting());
        M->EndBurst();
        TestTrue(TEXT("Ability cleanup preserves jump momentum"), M->Velocity.Equals(FVector(300, 200, JumpZ)));
        TestTrue(TEXT("Gravity applies immediately in falling mode"), M->IsFalling());
        TestEqual(TEXT("Cannot repeat Ctrl until landing"), M->CanBurst(true), ETripoAbilityFailure::AirUseSpent);
        TestFalse(TEXT("Second jump never enters constant-speed custom movement"), M->IsBursting());
        M->EndBurst();
        TestTrue(TEXT("Ability cleanup cannot restore velocity twice"), M->Velocity.Equals(FVector(300, 200, JumpZ)));
    }
    M->ResetAirUses();
    M->Velocity = FVector(300, 200, -500);
    M->BeginBurst(true, FVector::ForwardVector, 400, .2f);
    M->Velocity.Z = 0; // The ceiling collision path cancels vertical velocity.
    M->EndBurst();
    TestTrue(TEXT("Ceiling cancellation keeps horizontal momentum without upward relaunch"), M->Velocity.Equals(FVector(300, 200, 0)));
    GI->Shutdown();
    World->DestroyWorld(false);
    return true;
}
#endif
