#include "Misc/AutomationTest.h"
#include "Player/TripoMovementRules.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoJumpWindows, "Tripothon.Movement.JumpWindows", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoJumpWindows::RunTest(const FString&)
{
    using namespace TripoMovement;
    TestTrue(TEXT("Ledge grace allows fresh jump"), CanUseBufferedJump(10.,9.98,9.95,false,false,.12,.08));
    TestFalse(TEXT("Expired ledge grace rejects midair jump"), CanUseBufferedJump(10.,9.98,9.8,false,false,.12,.08));
    TestFalse(TEXT("Spent jump cannot be refreshed by coyote window"), CanUseBufferedJump(10.,9.98,9.99,false,true,.12,.08));
    TestTrue(TEXT("Landing consumes recent buffered input"), CanUseBufferedJump(10.,9.9,10.,true,false,.12,.08));
    TestFalse(TEXT("Old input cannot autojump on landing"), CanUseBufferedJump(10.,9.8,10.,true,false,.12,.08));
    TestFalse(TEXT("Future input is invalid"), CanUseBufferedJump(10.,10.1,10.,true,false,.12,.08));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoCameraLimits, "Tripothon.Movement.CameraLimits", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoCameraLimits::RunTest(const FString&)
{
    using namespace TripoMovement;
    TestEqual(TEXT("Anchor-relative right bound"), ClampYaw(160,90,35),125.f);
    TestEqual(TEXT("Anchor-relative left bound"), ClampYaw(10,90,35),55.f);
    TestEqual(TEXT("Wrap across 180 degrees stays within limit"), ClampYaw(-170,170,35),-170.f);
    TestEqual(TEXT("Wrapped angle clamps along shortest arc"), ClampYaw(-100,170,35),-155.f);
    return true;
}
#endif
