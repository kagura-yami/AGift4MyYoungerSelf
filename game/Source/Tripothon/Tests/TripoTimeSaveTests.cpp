#include "Misc/AutomationTest.h"
#include "Time/TripoHistoryComponent.h"
#include "Progress/TripoSaveGame.h"
#include "Story/TripoStoryCatalog.h"
#include "Kismet/GameplayStatics.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoHistoryTest, "Tripothon.Time.HistoryInterpolation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoHistoryTest::RunTest(const FString&)
{
    FTripoHistoryFrame A, B, Out; A.Time=10; B.Time=12; A.Transform.SetLocation(FVector(0,0,0)); B.Transform.SetLocation(FVector(200,0,0));
    A.Phase=.95; B.Phase=.05; TArray<FTripoHistoryFrame> Frames={A,B};
    TestTrue(TEXT("Sample by timestamp"), UTripoHistoryComponent::Sample(Frames,11,Out));
    TestEqual(TEXT("Interpolate position"), Out.Transform.GetLocation().X, 100.);
    TestTrue(TEXT("Phase crosses wrap through zero"), FMath::Abs(Out.Phase) < .001);
    Frames[0].BaseId=FGuid::NewGuid(); Frames[1].BaseId=FGuid::NewGuid();
    UTripoHistoryComponent::Sample(Frames,11,Out); TestFalse(TEXT("Different bases do not blend relative frames"),Out.BaseId.IsValid());
    TestFalse(TEXT("No frame means no playback"),UTripoHistoryComponent::Sample({},0,Out)); return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoSaveValidationTest, "Tripothon.Progress.SaveValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoSaveValidationTest::RunTest(const FString&)
{
    auto* S=NewObject<UTripoSaveGame>(); S->RunId=FGuid::NewGuid(); S->Levels.Init(0,8); S->MapPackage=TEXT("/Game/Maps/L_Tutorial");
    TestTrue(TEXT("Known schema valid"),S->IsValidData()); S->SchemaVersion=99;
    TestFalse(TEXT("Unknown schema rejected"),S->IsValidData()); S->SchemaVersion=1; S->Levels[2]=4;
    TestFalse(TEXT("Out-of-range ability level rejected"),S->IsValidData()); S->Levels[2]=0; S->MapPackage=TEXT("/Game/Maps/../External");
    TestFalse(TEXT("Invalid map path rejected"),S->IsValidData()); S->MapPackage=TEXT("/Game/Maps/L_Tutorial");
    S->ReplyChoice=3; TestFalse(TEXT("Invalid reply rejected"),S->IsValidData()); S->ReplyChoice=2;
    S->Completed.Add(TEXT("Home.Route1")); S->Viewed.Add(TEXT("Story.Finale.SendReply"));
    TArray<uint8> Bytes; TestTrue(TEXT("Save serializes"),UGameplayStatics::SaveGameToMemory(S,Bytes));
    auto* Loaded=Cast<UTripoSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    TestNotNull(TEXT("Save deserializes"),Loaded);
    if (Loaded) { TestTrue(TEXT("Roundtrip validates"),Loaded->IsValidData()); TestEqual(TEXT("Reply survives serialization"),Loaded->ReplyChoice,2); TestTrue(TEXT("Reward ledger survives serialization"),Loaded->Completed.Contains(TEXT("Home.Route1"))); }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoStoryCatalogTest, "Tripothon.Story.CatalogValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoStoryCatalogTest::RunTest(const FString&)
{
    auto* C=NewObject<UTripoStoryCatalog>(); FTripoStoryEvent E; E.Id=TEXT("Story.Test"); E.Text=FText::FromString(TEXT("Test")); E.GrantFloors.Init(0,8); C->Events.Add(E);
    TestTrue(TEXT("Valid event"),C->IsValidCatalog()); C->Events.Add(E); TestFalse(TEXT("Duplicate event rejected"),C->IsValidCatalog());
    C->Events.RemoveAt(1); C->Events[0].RequiredEvents.Add(TEXT("Missing")); TestFalse(TEXT("Missing prerequisite rejected"),C->IsValidCatalog());
    E.Id=TEXT("Story.Second"); E.RequiredEvents={TEXT("Story.Test")}; C->Events.Add(E); C->Events[0].RequiredEvents={E.Id};
    TestFalse(TEXT("Circular prerequisites rejected"),C->IsValidCatalog()); return true;
}
#endif
