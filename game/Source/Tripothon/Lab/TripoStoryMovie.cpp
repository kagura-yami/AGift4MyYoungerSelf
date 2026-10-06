#include "Lab/TripoHUD.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "MediaSource.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/GameViewportClient.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SOverlay.h"

bool ATripoHUD::PlayStoryMovie(UMediaSource* Source,UMaterialInterface* Material)
{
    if(bStoryMovie || !Source || !Material || !GetWorld()->GetGameViewport()) return false;
    StoryMoviePlayer=NewObject<UMediaPlayer>(this);
    StoryMoviePlayer->PlayOnOpen=false;
    StoryMoviePlayer->OnMediaOpened.AddDynamic(this,&ATripoHUD::StoryMovieOpened);
    StoryMoviePlayer->OnMediaOpenFailed.AddDynamic(this,&ATripoHUD::StoryMovieFailed);
    StoryMoviePlayer->OnEndReached.AddDynamic(this,&ATripoHUD::StoryMovieFinished);
    StoryMovieTexture=NewObject<UMediaTexture>(this);
    StoryMovieTexture->NewStyleOutput=true;
    StoryMovieTexture->SetMediaPlayer(StoryMoviePlayer); StoryMovieTexture->UpdateResource();
    StoryMovieMaterial=UMaterialInstanceDynamic::Create(Material,this);
    StoryMovieMaterial->SetTextureParameterValue(TEXT("MovieTexture"),StoryMovieTexture);
    StoryMovieBrush.DrawAs=ESlateBrushDrawType::Image; StoryMovieBrush.SetResourceObject(StoryMovieMaterial); StoryMovieBrush.ImageSize=FVector2D(1920,1080);
    const TWeakObjectPtr<ATripoHUD> Self(this);
    StoryMovieWidget=SNew(SOverlay)
        +SOverlay::Slot()[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor::Black).Padding(0)
            [SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[SNew(SBox).WidthOverride(1920).HeightOverride(1080)[SNew(SImage).Image(&StoryMovieBrush)]]]]
        +SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom).Padding(24)
            [SNew(SButton).OnClicked_Lambda([Self]{if(Self.IsValid())Self->StopStoryMovie();return FReply::Handled();})
                [SNew(STextBlock).Text(FText::FromString(TEXT("跳过  Esc")))]];
    bStoryMovie=true;
    GetWorld()->GetGameViewport()->AddViewportWidgetContent(StoryMovieWidget.ToSharedRef(),1000);
    // Block gameplay through the HUD while keeping media and timeout timers ticking.
    GetWorldTimerManager().SetTimer(StoryMovieTimeout,this,&ATripoHUD::StopStoryMovie,12.f,false);
    if(!StoryMoviePlayer->OpenSource(Source)) { StopStoryMovie(); return false; }
    return true;
}
void ATripoHUD::StoryMovieOpened(FString Url)
{
    if(!bStoryMovie || !StoryMoviePlayer) return;
    GetWorldTimerManager().ClearTimer(StoryMovieTimeout);
    if(!StoryMoviePlayer->Play()) { StopStoryMovie(); return; }
    GetWorldTimerManager().SetTimer(StoryMovieTimeout,this,&ATripoHUD::StopStoryMovie,
        FMath::Max(35.f,float(StoryMoviePlayer->GetDuration().GetTotalSeconds())+5.f),false);
}
void ATripoHUD::StoryMovieFailed(FString Url)
{
    UE_LOG(LogTemp,Warning,TEXT("Story movie failed to open: %s"),*Url);
    StopStoryMovie();
}
void ATripoHUD::StoryMovieFinished()
{
    if(!bStoryMovie) return;
    const bool bFinalChapter=UGameplayStatics::GetCurrentLevelName(this,true)==TEXT("lv4");
    StopStoryMovie();
    if(bFinalChapter) UGameplayStatics::OpenLevel(this,TEXT("/Game/Maps/L_MainMenu"));
}
void ATripoHUD::StopStoryMovie()
{
    GetWorldTimerManager().ClearTimer(StoryMovieTimeout);
    if(StoryMoviePlayer) { StoryMoviePlayer->OnEndReached.RemoveAll(this); StoryMoviePlayer->OnMediaOpened.RemoveAll(this); StoryMoviePlayer->OnMediaOpenFailed.RemoveAll(this); StoryMoviePlayer->Close(); }
    if(StoryMovieWidget.IsValid() && GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(StoryMovieWidget.ToSharedRef());
    StoryMovieWidget.Reset();

    bStoryMovie=false; PanelKey.Empty();
    StoryMovieBrush.SetResourceObject(nullptr);
    StoryMovieMaterial=nullptr; StoryMovieTexture=nullptr; StoryMoviePlayer=nullptr;
}
