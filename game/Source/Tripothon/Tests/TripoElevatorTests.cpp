#include "Misc/AutomationTest.h"
#include "World/TripoElevator.h"
#include "World/TripoChaseNavigation.h"
#include "World/TripoChaseHideZone.h"
#include "Progress/TripoSaveGame.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoElevatorRestoreTest,"Tripothon.Lv4.ElevatorRestore",EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoElevatorRestoreTest::RunTest(const FString&)
{
    auto* World=UWorld::CreateWorld(EWorldType::Game,false);
    auto* Lift=World->SpawnActor<ATripoElevator>();
    Lift->Stop1=FVector(0,0,750); Lift->Phase=ETripoElevatorPhase::Moving; Lift->DoorAlpha=.6;
    TestFalse(TEXT("Moving lift forbids world save"),ATripoElevator::AllStable(World));
    Lift->RestoreFloor(1);
    TestTrue(TEXT("Restore docks at recorded stop"),Lift->Cabin->GetRelativeLocation().Equals(Lift->Stop1));
    TestEqual(TEXT("Restore closes doors"),Lift->DoorAlpha,0.f);
    TestTrue(TEXT("Restore clears travel request"),Lift->IsStable());
    TestEqual(TEXT("Absent floor remains physically blocked"),Lift->LandingBarriers[0]->GetCollisionEnabled(),ECollisionEnabled::QueryAndPhysics);
    TestTrue(TEXT("Docked lift allows save"),ATripoElevator::AllStable(World));
    Lift->RestoreFloor(0);
    TestEqual(TEXT("Upper landing now blocked"),Lift->LandingBarriers[1]->GetCollisionEnabled(),ECollisionEnabled::QueryAndPhysics);
    World->DestroyWorld(false); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoElevatorSaveTest,"Tripothon.Lv4.ElevatorSaveCompatibility",EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoElevatorSaveTest::RunTest(const FString&)
{
    auto* S=NewObject<UTripoSaveGame>(); S->RunId=FGuid::NewGuid(); S->Levels.Init(0,8); S->MapPackage=TEXT("/Game/Maps/L_Lv4MechanismWhitebox");
    TestTrue(TEXT("Old schema without elevator entries remains valid"),S->IsValidData());
    const FGuid Id=FGuid::NewGuid(); S->ElevatorFloors.Add(Id,1);
    TArray<uint8> Bytes; TestTrue(TEXT("Serialize elevator floor"),UGameplayStatics::SaveGameToMemory(S,Bytes));
    auto* Loaded=Cast<UTripoSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    TestTrue(TEXT("Floor survives real serialization"),Loaded && Loaded->IsValidData() && Loaded->ElevatorFloors.FindRef(Id)==1);
    S->ElevatorFloors[Id]=2; TestFalse(TEXT("Reject invalid stop"),S->IsValidData());
    S->ElevatorFloors.Empty(); S->ElevatorFloors.Add(FGuid(),0); TestFalse(TEXT("Reject missing stable ID"),S->IsValidData());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoHideNavigationTest,"Tripothon.Lv4.HideNavigation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoHideNavigationTest::RunTest(const FString&)
{
    auto* World=UWorld::CreateWorld(EWorldType::Game,false); auto* Zone=World->SpawnActor<ATripoChaseHideZone>();
    TestEqual(TEXT("Players can enter"),Zone->Volume->GetCollisionResponseToChannel(ECC_Pawn),ECR_Ignore);
    TestEqual(TEXT("Monster capsule cannot enter"),Zone->Volume->GetCollisionResponseToChannel(ECC_GameTraceChannel1),ECR_Block);
    const auto* Filter=GetDefault<UTripoChaserNavFilter>();
    TestTrue(TEXT("Only chaser navigation excludes safe area"),Filter->Areas.Num()==1 && Filter->Areas[0].bIsExcluded && Filter->Areas[0].AreaClass==UTripoHideNavArea::StaticClass());
    Zone->SetEnabled(false); TestEqual(TEXT("Disabled zone removes physical barrier"),Zone->Volume->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
    World->DestroyWorld(false); return true;
}
#endif
