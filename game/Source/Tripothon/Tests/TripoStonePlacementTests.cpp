#include "Misc/AutomationTest.h"
#include "Player/TripoCharacter.h"
#include "Abilities/TripoWorldAbilities.h"
#include "Abilities/TripoStone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoStonePlacementTest,"Tripothon.Stone.DistanceWallAndShatter",EAutomationTestFlags::EditorContext|EAutomationTestFlags::ProductFilter)
bool FTripoStonePlacementTest::RunTest(const FString&)
{
    UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
    auto* GI=NewObject<UGameInstance>(); World->SetGameInstance(GI); GI->Init();
    auto* Player=World->SpawnActor<ATripoCharacter>(FVector(0,0,200),FRotator::ZeroRotator);
    Player->AdjustStoneDistance(100); TestEqual(TEXT("Wheel clamps maximum"),Player->StonePlacementOffset.X,600.);
    Player->AdjustStoneDistance(-100); TestEqual(TEXT("Wheel clamps minimum"),Player->StonePlacementOffset.X,120.);
    Player->AdjustStoneDistance(1); TestEqual(TEXT("Wheel uses configured step"),Player->StonePlacementOffset.X,150.);
    Player->StonePlacementOffset.X=600;
    FTripoAbilityParameters P; P.Capacity=2; FVector Location;
    TestEqual(TEXT("Clear path allows distant placement"),UTripoStoneAbility::CheckPlacement(Player,P,Location),ETripoAbilityFailure::None);
    TestTrue(TEXT("Clear path reaches requested distance"),Location.X>590);
    auto* Stair=World->SpawnActor<AActor>();
    auto* Riser=NewObject<UBoxComponent>(Stair); Stair->SetRootComponent(Riser); Stair->AddInstanceComponent(Riser);
    Riser->SetBoxExtent(FVector(15,200,75)); Riser->SetCollisionProfileName(TEXT("BlockAll")); Riser->RegisterComponent();
    Stair->SetActorLocation(FVector(-65,0,75));
    TestEqual(TEXT("Nearby raised stair behind feet does not invalidate clear target"),UTripoStoneAbility::CheckPlacement(Player,P,Location),ETripoAbilityFailure::None);
    TestTrue(TEXT("Stair start overlap does not shorten a clear placement"),Location.X>590);
    auto* Wall=World->SpawnActor<AActor>();
    auto* Box=NewObject<UBoxComponent>(Wall); Wall->SetRootComponent(Box); Wall->AddInstanceComponent(Box);
    Box->SetBoxExtent(FVector(5,400,400)); Box->SetCollisionProfileName(TEXT("BlockAll")); Box->RegisterComponent();
    Wall->SetActorLocation(FVector(300,0,200));
    TestEqual(TEXT("Wall-facing placement succeeds"),UTripoStoneAbility::CheckPlacement(Player,P,Location),ETripoAbilityFailure::None);
    TestTrue(TEXT("Entire stone stays on near side of thin wall"),Location.X+75<295 && Location.X>100);
    auto* Stone=World->SpawnActor<ATripoStone>(FVector(-300,0,200),FRotator::ZeroRotator); Stone->SetOwner(Player);
    Stone->Shatter(); TestFalse(TEXT("Break releases capacity immediately"),Stone->IsUsable());
    TestEqual(TEXT("Broken body cannot block player"),Stone->Body->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
    TArray<UStaticMeshComponent*> Pieces; Stone->GetComponents(Pieces);
    TestEqual(TEXT("Nine cosmetic fragments plus original body"),Pieces.Num(),10);
    Stone->Shatter(); Pieces.Reset(); Stone->GetComponents(Pieces);
    TestEqual(TEXT("Repeated break does not duplicate fragments"),Pieces.Num(),10);
    GI->Shutdown(); World->DestroyWorld(false); return true;
}
#endif
