#include "Misc/AutomationTest.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Misc/PackageName.h"
#include "Misc/ConfigCacheIni.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTripoChapterConfigTest,"Tripothon.Flow.ChapterPackages",EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FTripoChapterConfigTest::RunTest(const FString&)
{
    const auto* Flow=GetDefault<UTripoProgressSubsystem>();
    TArray<FString> Cooked;
    GConfig->GetArray(TEXT("/Script/UnrealEd.ProjectPackagingSettings"),TEXT("MapsToCook"),Cooked,GGameIni);
    for (const FString& Map : {Flow->FirstChapterMap,Flow->SecondChapterMap,Flow->ThirdChapterMap}) {
        TestTrue(TEXT("Configured chapter package exists"),FPackageName::DoesPackageExist(Map));
        TestTrue(TEXT("Chapter included in packaged build"),Cooked.ContainsByPredicate([&](const FString& Entry){return Entry.Contains(Map); }));
    }
    TestTrue(TEXT("First and second chapters are distinct"),Flow->FirstChapterMap!=Flow->SecondChapterMap);
    TestTrue(TEXT("First and third chapters are distinct"),Flow->FirstChapterMap!=Flow->ThirdChapterMap);
    return true;
}
#endif
