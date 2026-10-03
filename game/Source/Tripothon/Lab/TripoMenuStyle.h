#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Fonts/CompositeFont.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

// Fonts are staged as UFS files so Slate also loads them from packaged games.
namespace TripoMenu
{
inline const FLinearColor Ink(.035f,.065f,.045f);
inline const FLinearColor Muted(.19f,.235f,.18f);
inline const FLinearColor Brass(.34f,.20f,.075f);
inline const FLinearColor Paper(.95f,.92f,.82f);

inline FSlateFontInfo Font(int32 Size, bool bTitle=false)
{
    static const TSharedPtr<const FCompositeFont> Sans=MakeShared<FCompositeFont>(NAME_None,
        FPaths::ProjectContentDir()/TEXT("UI/Fonts/SourceHanSansSC-Medium.otf"),EFontHinting::Default,EFontLoadingPolicy::LazyLoad);
    static const TSharedPtr<const FCompositeFont> Serif=MakeShared<FCompositeFont>(NAME_None,
        FPaths::ProjectContentDir()/TEXT("UI/Fonts/SourceHanSerifSC-Bold.otf"),EFontHinting::Default,EFontLoadingPolicy::LazyLoad);
    FSlateFontInfo Result(bTitle?Serif:Sans,Size);
    Result.LetterSpacing=bTitle?30:0;
    return Result;
}
inline TSharedRef<STextBlock> Label(const FString& Value,int32 Size=18,FLinearColor Color=Ink,bool bTitle=false,bool bWrap=true)
{
    return SNew(STextBlock).Font(Font(Size,bTitle)).ColorAndOpacity(Color)
        .Text(FText::FromString(Value)).AutoWrapText(bWrap && !bTitle).LineHeightPercentage(1.22f);
}
inline TSharedRef<SWidget> Rule()
{
    return SNew(SBox).HeightOverride(1)
        [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(.26f,.22f,.12f,.25f)).Padding(0)];
}
inline TSharedRef<SWidget> Heading(const FString& Title,const FString& Subtitle=FString())
{
    auto Column=SNew(SVerticalBox);
    Column->AddSlot().AutoHeight()[Label(Title,32,Ink,true)];
    if (!Subtitle.IsEmpty()) Column->AddSlot().AutoHeight().Padding(0,8,0,0)[Label(Subtitle,15,Muted)];
    Column->AddSlot().AutoHeight().Padding(0,12,0,12)[Rule()];
    return Column;
}
inline TSharedRef<SWidget> Keycap(const FString& Key)
{
    static const FSlateRoundedBoxBrush Base(FLinearColor(.18f,.22f,.18f),4.f);
    static const FSlateRoundedBoxBrush Face(FLinearColor(.94f,.91f,.81f),3.f);
    return SNew(SBox).MinDesiredWidth(32).HeightOverride(30)
        [SNew(SBorder).BorderImage(&Base).Padding(FMargin(1,1,1,3))
            [SNew(SBorder).BorderImage(&Face).HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(FMargin(7,0))
                [Label(Key,13,Ink,false,false)]]];
}
}
