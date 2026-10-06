#include "Lab/TripoHUD.h"
#include "Lab/TripoMenuStyle.h"
#include "Player/TripoPlayerSettings.h"
#include "GameFramework/GameUserSettings.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

TSharedRef<SWidget> ATripoHUD::BuildSettingsControls()
{
    const TWeakObjectPtr<ATripoHUD> Weak(this);
    auto* Preferences=UTripoPlayerSettings::Get();
    auto* Video=UGameUserSettings::GetGameUserSettings();
    auto Page=SNew(SVerticalBox);
    Page->AddSlot().AutoHeight()[TripoMenu::Heading(TEXT("游戏设置"),TEXT("修改立即生效，自动保存到本机。"))];
    auto Row=[&](const TCHAR* Name,TSharedRef<SWidget> Control)
    {
        Page->AddSlot().AutoHeight().Padding(0,3)
            [SNew(SHorizontalBox)
                +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[TripoMenu::Label(Name,16,TripoMenu::Ink,true)]
                +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[SNew(SBox).WidthOverride(330).HeightOverride(32)[Control]]];
    };
    auto ValueButton=[&](TFunction<FString()> Text,TFunction<void()> Change)
    {
        return SNew(SButton).ButtonStyle(&MenuButtonStyle()).ButtonColorAndOpacity(FLinearColor(.25f,.28f,.17f,.12f))
            .ContentPadding(FMargin(8,0)).HAlign(HAlign_Center).VAlign(VAlign_Center)
            .OnClicked_Lambda([Change]{Change();return FReply::Handled();})
            [SNew(STextBlock).Font(TripoMenu::Font(16)).ColorAndOpacity(TripoMenu::Ink).Text_Lambda([Text]{return FText::FromString(Text());})];
    };
    auto Slider=[&](TFunction<float()> Value,TFunction<void(float)> Change,TFunction<FString()> Text)
    {
        return SNew(SHorizontalBox)
            +SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(0,0,12,0)
                [SNew(SSlider).Value_Lambda([Value]{return Value();}).StepSize(.01f)
                    .SliderBarColor(FLinearColor(.30f,.37f,.25f)).SliderHandleColor(FLinearColor(.55f,.34f,.12f))
                    .OnValueChanged_Lambda([Change](float V){Change(V);})]
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                [SNew(SBox).WidthOverride(70)[SNew(STextBlock).Font(TripoMenu::Font(15)).ColorAndOpacity(TripoMenu::Ink)
                    .Text_Lambda([Text]{return FText::FromString(Text());})]];
    };
    Row(TEXT("总音量"),Slider([Preferences]{return Preferences->MasterVolume;},[Preferences,Weak](float V){if(Weak.IsValid()) Preferences->SetVolume(V,Weak.Get());},
        [Preferences]{return FString::Printf(TEXT("%d%%"),FMath::RoundToInt(Preferences->MasterVolume*100));}));
    Row(TEXT("鼠标灵敏度"),Slider([Preferences]{return (Preferences->MouseSensitivity-.25f)/2.75f;},[Preferences](float V){Preferences->SetSensitivity(.25f+V*2.75f);},
        [Preferences]{return FString::Printf(TEXT("%.2f"),Preferences->MouseSensitivity);}));
    Row(TEXT("反转纵向视角"),ValueButton([Preferences]{return Preferences->bInvertLookY?TEXT("开启"):TEXT("关闭");},
        [Preferences]{Preferences->SetInvertY(!Preferences->bInvertLookY);}));
    Page->AddSlot().AutoHeight().Padding(0,4)[TripoMenu::Rule()];
    Row(TEXT("画面质量"),ValueButton([Video]{const int32 Q=Video->GetOverallScalabilityLevel();const TCHAR* Names[]={TEXT("低"),TEXT("中"),TEXT("高"),TEXT("极高")};return Q>=0&&Q<4?FString(Names[Q]):FString(TEXT("自定义"));},
        [Video]{Video->SetOverallScalabilityLevel((Video->GetOverallScalabilityLevel()+1)%4);Video->ApplyNonResolutionSettings();Video->SaveSettings();}));
    Row(TEXT("帧率上限"),ValueButton([Video]{float V=Video->GetFrameRateLimit();return V<=0?FString(TEXT("不限")):FString::Printf(TEXT("%d FPS"),FMath::RoundToInt(V));},
        [Video]{float V=Video->GetFrameRateLimit();Video->SetFrameRateLimit(V==30?60:V==60?120:V==120?0:30);Video->ApplyNonResolutionSettings();Video->SaveSettings();}));
    Row(TEXT("垂直同步"),ValueButton([Video]{return Video->IsVSyncEnabled()?TEXT("开启"):TEXT("关闭");},
        [Video]{Video->SetVSyncEnabled(!Video->IsVSyncEnabled());Video->ApplyNonResolutionSettings();Video->SaveSettings();}));

    Page->AddSlot().AutoHeight().Padding(0,5,0,0)[ValueButton([]{return FString(TEXT("恢复默认设置"));},[Preferences,Video,Weak]{
        Preferences->SetVolume(1.f,Weak.Get());Preferences->SetSensitivity(1.5f);Preferences->SetInvertY(false);
        Video->SetOverallScalabilityLevel(2);Video->SetFrameRateLimit(60);Video->SetVSyncEnabled(false);Video->ApplyNonResolutionSettings();Video->SaveSettings();})];
    return Page;
}
