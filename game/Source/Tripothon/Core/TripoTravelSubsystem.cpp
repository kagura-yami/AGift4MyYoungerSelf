#include "Core/TripoTravelSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "MoviePlayer.h"
#include "Lab/TripoMenuStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Brushes/SlateDynamicImageBrush.h"
void UTripoTravelSubsystem::Initialize(FSubsystemCollectionBase& C)
{
    Super::Initialize(C);
    LoadHandle=FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this,&UTripoTravelSubsystem::Loaded);
}
void UTripoTravelSubsystem::Deinitialize()
{
    FTSTicker::GetCoreTicker().RemoveTicker(Ticker);
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(LoadHandle);
    if(Overlay && GetGameInstance()->GetGameViewportClient()) GetGameInstance()->GetGameViewportClient()->RemoveViewportWidgetContent(Overlay.ToSharedRef());
    Overlay.Reset(); Super::Deinitialize();
}
TSharedRef<SWidget> UTripoTravelSubsystem::MakeScreen(bool bMovie)
{
    const bool bSchool=Destination.ToString().Contains(TEXT("School"));
    auto Titles=SNew(SVerticalBox);
    Titles->AddSlot().AutoHeight()[TripoMenu::Label(bSchool?TEXT("第二章"):TEXT("下一站"),22,FLinearColor(.78,.66,.43))];
    Titles->AddSlot().AutoHeight().Padding(0,18,0,28)[TripoMenu::Label(bSchool?TEXT("学校"):TEXT("继续旅程"),76,TripoMenu::Paper,true)];
    Titles->AddSlot().AutoHeight()[SNew(SBox).WidthOverride(60).HeightOverride(2).HAlign(HAlign_Left)
        [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.78,.66,.43)).Padding(0)]];
    auto Footer=SNew(SHorizontalBox);
    Footer->AddSlot().FillWidth(1)[TripoMenu::Label(TEXT("明天寄来的礼物"),20,FLinearColor(.72,.70,.63),true)];
    Footer->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,18,0)[SNew(SThrobber).NumPieces(3)];
    Footer->AddSlot().AutoWidth().VAlign(VAlign_Center)[TripoMenu::Label(TEXT("正在前往下一站"),18,TripoMenu::Paper)];
    auto Layout=SNew(SOverlay)
        +SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(140,0,0,30)[Titles]
        +SOverlay::Slot().VAlign(VAlign_Bottom).Padding(140,0,140,80)[Footer];
    auto Screen=SNew(SOverlay)
        +SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(.018,.016,.014,1)).Padding(0)]
        +SOverlay::Slot()[SNew(SScaleBox).Stretch(EStretch::ScaleToFill)
            [SNew(SBox).WidthOverride(1920).HeightOverride(1080)[SNew(SImage).Image(LoadingArt.Get())]]]
        +SOverlay::Slot()[SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
            [SNew(SBox).WidthOverride(1920).HeightOverride(1080)[Layout]]];
    if(!bMovie) Screen->SetRenderOpacity(0.f);
    return Screen;
}
void UTripoTravelSubsystem::Travel(FName Map)
{
    if(Overlay) return;
    Destination=Map;
    if(!LoadingArt) LoadingArt=MakeShared<FSlateDynamicImageBrush>(FName(*(FPaths::ProjectContentDir()/TEXT("UI/Loading/SchoolCorridor.png"))),FVector2D(1920,1080));
    Alpha=0; Elapsed=0; bOpening=false; bArrived=false;
    if(auto* PC=GetGameInstance()->GetFirstLocalPlayerController()) { PC->SetIgnoreMoveInput(true); PC->SetIgnoreLookInput(true); }
    Overlay=MakeScreen(false);
    if(auto* V=GetGameInstance()->GetGameViewportClient()) V->AddViewportWidgetContent(Overlay.ToSharedRef(),10000);
    Ticker=FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this,&UTripoTravelSubsystem::Update));
}
void UTripoTravelSubsystem::Loaded(UWorld* World)
{
    if(!Overlay || !World || World->GetGameInstance()!=GetGameInstance()) return;
    bArrived=true; Elapsed=0;
}
bool UTripoTravelSubsystem::Update(float Dt)
{
    Elapsed+=FMath::Min(Dt,.05f);
    if(!bOpening)
    {
        Alpha=FMath::Clamp(Elapsed/.35f,0.f,1.f);
        if(Elapsed>=.4f)
        {
            bOpening=true;
            // MoviePlayer keeps Slate animating while the game thread loads in standalone/builds.
            // PIE uses the same persistent viewport overlay instead.
            if(GetWorld()->WorldType!=EWorldType::PIE && GetMoviePlayer())
            {
                FLoadingScreenAttributes A; A.WidgetLoadingScreen=MakeScreen(true);
                A.MinimumLoadingScreenDisplayTime=.3f; A.bAutoCompleteWhenLoadingCompletes=true;
                GetMoviePlayer()->SetupLoadingScreen(A);
            }
            UGameplayStatics::OpenLevel(GetGameInstance(),Destination);
        }
    }
    else if(bArrived && Elapsed>.25f)
    {
        Alpha=FMath::Clamp(1.f-(Elapsed-.25f)/.45f,0.f,1.f);
        if(Alpha<=0)
        {
            if(auto* V=GetGameInstance()->GetGameViewportClient()) V->RemoveViewportWidgetContent(Overlay.ToSharedRef());
            Overlay.Reset();
            if(auto* PC=GetGameInstance()->GetFirstLocalPlayerController()) { PC->ResetIgnoreMoveInput(); PC->ResetIgnoreLookInput(); }
            return false;
        }
    }
    if(Overlay) Overlay->SetRenderOpacity(Alpha);
    return true;
}
