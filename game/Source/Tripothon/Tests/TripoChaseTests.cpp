#include "Misc/AutomationTest.h"
#include "World/TripoChaser.h"
#include "World/TripoChaseHideZone.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoChaseSpeedTest,"Tripothon.Chase.SpeedBalance",EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoChaseSpeedTest::RunTest(const FString&)
{
    auto Speed = [](float Walk,float Aggro,bool Air,bool Chase) { return ATripoChaser::CalculateSpeed(Walk,Aggro,Air,Chase,.92f,.08f,.85f,.55f); };
    TestEqual(TEXT("Full aggro capped to normal running speed"),Speed(540,1,false,true),540.f);
    TestTrue(TEXT("Low aggro leaves room to escape"),Speed(540,0,false,true)<540.f);
    TestTrue(TEXT("Jumping does not accelerate the pursuer"),Speed(540,1,true,true)<Speed(540,1,false,true));
    TestTrue(TEXT("Search is slower than pursuit"),Speed(540,1,false,false)<Speed(540,0,false,true));
    TestEqual(TEXT("Aggro over maximum is bounded"),Speed(540,10,false,true),540.f);
    TestEqual(TEXT("Different normal speed scales correctly"),Speed(300,1,false,true),300.f);
    TestEqual(TEXT("Negative speed cannot reverse NPC"),Speed(-1,1,false,true),0.f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoChaseHideTest,"Tripothon.Chase.HideVolume",EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoChaseHideTest::RunTest(const FString&)
{
    auto* World = UWorld::CreateWorld(EWorldType::Game,false);
    auto* Zone = World->SpawnActor<ATripoChaseHideZone>();
    Zone->Volume->SetBoxExtent(FVector(200,50,100));
    Zone->SetActorLocationAndRotation(FVector(500,300,100),FRotator(0,90,0));
    TestTrue(TEXT("Rotated zone contains its centre"),Zone->ContainsPoint(FVector(500,300,100)));
    TestTrue(TEXT("Rotated long axis applies"),Zone->ContainsPoint(FVector(500,450,100)));
    TestFalse(TEXT("Outside rotated short axis is exposed"),Zone->ContainsPoint(FVector(600,300,100)));
    TestFalse(TEXT("Jumping above zone is exposed"),Zone->ContainsPoint(FVector(500,300,250)));
    Zone->bEnabled = false;
    TestFalse(TEXT("Disabled hiding zone does not protect"),Zone->ContainsPoint(FVector(500,300,100)));
    World->DestroyWorld(false);
    return true;
}
#endif
