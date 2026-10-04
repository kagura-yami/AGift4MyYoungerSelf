#include "InputKeyEventArgs.h"
#include "Components/CapsuleComponent.h"
#include "Misc/AutomationTest.h"
#include "Time/TripoEchoActor.h"
#include "Player/TripoPlayerController.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoEchoControlTest,"Tripothon.Time.PersistentEchoControl",EAutomationTestFlags::EditorContext|EAutomationTestFlags::ProductFilter)
bool FTripoEchoControlTest::RunTest(const FString&)
{
    auto* W=UWorld::CreateWorld(EWorldType::Game,false);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(W);
    auto* GI=NewObject<UGameInstance>(); W->SetGameInstance(GI); GI->Init();
    auto* P=W->SpawnActor<ATripoCharacter>(); P->Abilities->InitializeDefinitions();
    auto* PC=W->SpawnActor<ATripoPlayerController>(); PC->SpawnPlayerCameraManager(); PC->Possess(P);
    TestFalse(TEXT("Locked clone cannot spawn"),PC->CreateEcho());
    P->Abilities->GrantLevelFloor(ETripoAbility::Echo,1);
    TestTrue(TEXT("Create without history"),PC->CreateEcho());
    TestEqual(TEXT("Level one capacity"),PC->GetEchoCapacity(),1);
    TestFalse(TEXT("Cannot exceed capacity"),PC->CreateEcho());
    auto* First=Cast<ATripoEchoActor>(PC->GetEchoBodies().Last());
    TestEqual(TEXT("Same location"),First->GetActorLocation(),P->GetActorLocation());
    TestEqual(TEXT("No expiry"),First->GetLifeSpan(),0.f);
    TestEqual(TEXT("Clone cannot retract another body camera"),First->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Camera),ECR_Ignore);
    P->Abilities->GrantLevelFloor(ETripoAbility::Echo,2);
    TestTrue(TEXT("Second level creates second clone"),PC->CreateEcho());
    TestFalse(TEXT("Second level capped"),PC->CreateEcho());
    P->Abilities->GrantLevelFloor(ETripoAbility::Echo,3);
    TestTrue(TEXT("Third level creates third clone"),PC->CreateEcho());
    auto* Third=Cast<ATripoEchoActor>(PC->GetEchoBodies().Last());
    TestEqual(TEXT("Three clones"),PC->GetEchoCount(),3);
    TestFalse(TEXT("No fourth clone"),PC->CreateEcho());
    PC->KeyForTest(EKeys::C,true); PC->KeyForTest(EKeys::C,false);
    TestTrue(TEXT("C tap selects oldest clone without creating"),PC->GetPawn()==First && PC->GetEchoCount()==3);
    PC->KeyForTest(EKeys::C,true); PC->KeyForTest(EKeys::C,false);
    TestTrue(TEXT("C tap selects next clone"),PC->GetPawn()==PC->GetEchoBodies()[2]);
    PC->KeyForTest(EKeys::C,true); PC->KeyForTest(EKeys::C,false);
    TestTrue(TEXT("C tap reaches third clone"),PC->GetPawn()==Third);
    PC->KeyForTest(EKeys::C,true); PC->KeyForTest(EKeys::C,false);
    TestTrue(TEXT("C tap wraps to original"),PC->GetPawn()==P);
    TestTrue(TEXT("Open switch wheel"),PC->OpenEchoWheel());
    PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,48.f)); // Actual mouse input path: top original, right first clone.
    PC->PlayerCameraManager->UpdateCamera(.2f);
    TestTrue(TEXT("Preview changes only view"),PC->GetPawn()==P && PC->GetViewTarget()==First);
    TestEqual(TEXT("Right selects first clone"),PC->GetWheelSelection(),1);
    PC->CloseEchoWheel(false);
    TestTrue(TEXT("Cancel restores original view and pawn"),PC->GetPawn()==P && PC->GetViewTarget()==P);
    PC->OpenEchoWheel(); PC->MoveEchoWheel(FVector2D(120,0)); PC->CloseEchoWheel(true);
    TestTrue(TEXT("Release commits possession"),PC->GetPawn()==First);
    First->SetActorLocation(FVector(400,0,200));
    PC->OpenEchoWheel(); PC->CloseEchoWheel(true);
    TestTrue(TEXT("Center release cancels"),PC->GetPawn()==First);
    TestTrue(TEXT("Short recall always removes newest"),PC->ReclaimEcho());
    TestTrue(TEXT("Third destroyed"),Third->IsActorBeingDestroyed());
    TestTrue(TEXT("Older controlled clone remains controlled"),PC->GetPawn()==First);
    PC->OpenEchoWheel(true); PC->MoveEchoWheel(FVector2D(0,-120));
    TestTrue(TEXT("Recall preview does not destroy"),!First->IsActorBeingDestroyed());
    PC->CloseEchoWheel(true);
    TestTrue(TEXT("Selected controlled clone is reclaimed"),First->IsActorBeingDestroyed());
    TestTrue(TEXT("Returns to original before destruction"),PC->GetPawn()==P);
    TestEqual(TEXT("One remaining"),PC->GetEchoCount(),1);
    auto* Foreign=W->SpawnActor<ATripoEchoActor>();
    TestFalse(TEXT("Cannot reclaim unrelated actor"),PC->ReclaimEcho(Foreign));
    TestFalse(TEXT("Original survives recall"),P->IsActorBeingDestroyed());
    PC->OpenEchoWheel(); PC->MoveEchoWheel(FVector2D(0,120)); PC->FlushPressedKeys();
    TestFalse(TEXT("Focus loss cancels wheel"),PC->IsEchoWheelOpen());
    TestTrue(TEXT("Focus loss restores camera without possessing"),PC->GetPawn()==P && PC->GetViewTarget()==P);
    GI->Shutdown(); GEngine->DestroyWorldContext(W); W->DestroyWorld(false); return true;
}
#endif
