#pragma once
#include "Lab/TripoMenuStyle.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Brushes/SlateDynamicImageBrush.h"

// Owns its cropped paper brush and animation state for the lifetime of the card.
class STripoSkillCard : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STripoSkillCard) {}
        SLATE_ARGUMENT(int32, Ability)
        SLATE_ARGUMENT(const FSlateBrush*, Icon)
        SLATE_ARGUMENT(FString, Title)
        SLATE_ARGUMENT(FString, Description)
        SLATE_ARGUMENT(TSharedPtr<bool>, Chosen)
        SLATE_EVENT(FSimpleDelegate, OnSelected)
    SLATE_END_ARGS()
    void Construct(const FArguments& A)
    {
        Chosen=A._Chosen; Selected=A._OnSelected;
        static const auto Atlas=MakeShared<FSlateDynamicImageBrush>(FName(*(FPaths::ProjectContentDir()/TEXT("UI/SkillCards/SkillCardAtlas.png"))),FVector2D(2400,2160));
        Paper=*Atlas; Paper.DrawAs=ESlateBrushDrawType::Image;
        const int32 Index=FMath::Clamp(A._Ability,0,7);
        Paper.SetUVRegion(FBox2f(FVector2f((Index%4)/4.f,(Index/4)/2.f),FVector2f((Index%4+1)/4.f,(Index/4+1)/2.f)));
        Birth=FPlatformTime::Seconds();
        static const FButtonStyle Invisible=FButtonStyle().SetNormal(FSlateNoResource()).SetHovered(FSlateNoResource()).SetPressed(FSlateNoResource());
        static const FSlateRoundedBoxBrush Outline(FLinearColor::Transparent,2.f,FLinearColor(.85,.66,.32),1.5f);
        auto Text=SNew(SVerticalBox);
        Text->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(0,0,0,9)[TripoMenu::Label(A._Title,23,TripoMenu::Ink,true)];
        Text->AddSlot().AutoHeight()[TripoMenu::Label(A._Description,13,TripoMenu::Ink)];
        Text->AddSlot().FillHeight(1);
        ChildSlot[SNew(SBox).WidthOverride(312).HeightOverride(606)
            [SAssignNew(Button,SButton).ButtonStyle(&Invisible).ContentPadding(0)
                .OnClicked_Lambda([this]{
                    if (*Chosen) return FReply::Handled();
                    *Chosen=true; bSelected=true;
                    RegisterActiveTimer(.38f,FWidgetActiveTimerDelegate::CreateLambda([this](double,float){Selected.ExecuteIfBound();return EActiveTimerReturnType::Stop;}));
                    return FReply::Handled();
                })
                [SNew(SOverlay)
                    +SOverlay::Slot().Padding(0,0,0,46)[SNew(SImage).Image(&Paper)]
                    +SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top).Padding(22,22)[SNew(SBox).WidthOverride(34).HeightOverride(34)[SNew(SImage).Image(A._Icon)]]
                    +SOverlay::Slot().Padding(32,384,32,110)[Text]
                    +SOverlay::Slot().Padding(2,2,2,48)[SNew(SBorder).BorderImage(&Outline).Visibility(EVisibility::HitTestInvisible)
                        .BorderBackgroundColor_Lambda([this]{return FLinearColor(1,1,1,FMath::Clamp(Lift,0.f,1.f));})]
                    +SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0,0,0,7)
                        [SNew(STextBlock).Font(TripoMenu::Font(15,true)).ColorAndOpacity(TripoMenu::Paper)
                            .Text_Lambda([this]{return FText::FromString(bSelected?TEXT("已收下  ✓"):TEXT("收下这份礼物  →"));})]
                ]]];
        SetRenderTransformPivot(FVector2D(.5,.5));
    }
    virtual void Tick(const FGeometry& G,double T,float Dt) override
    {
        SCompoundWidget::Tick(G,T,Dt);
        const float Target=bSelected?1.f:Button->IsPressed()?-.35f:(Button->IsHovered() || Button->HasKeyboardFocus()) && !*Chosen?1.f:0.f;
        Lift=FMath::FInterpTo(Lift,Target,Dt,12.f);
        SetRenderTransform(FSlateRenderTransform(FScale2D(1.f+Lift*.025f),FVector2D(0,-Lift*12.f)));
        SetRenderOpacity(FMath::Clamp(float((T-Birth)/.3),0.f,1.f)*(*Chosen && !bSelected?.38f:1.f));
    }
private:
    FSlateBrush Paper;
    TSharedPtr<SButton> Button;
    TSharedPtr<bool> Chosen;
    FSimpleDelegate Selected;
    double Birth=0;
    float Lift=0;
    bool bSelected=false;
};
