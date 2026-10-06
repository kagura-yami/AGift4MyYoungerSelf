#pragma once
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"

// Resolution-independent engraved emblems, all drawn on the same 72px grid.
class STripoGuideIcon : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STripoGuideIcon) {} SLATE_ARGUMENT(int32, Kind) SLATE_END_ARGS()
    void Construct(const FArguments& Args) { Kind=Args._Kind; }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(72,72); }
    virtual int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool) const override
    {
        const FLinearColor Gold=FLinearColor(.38f,.24f,.10f)*Style.GetColorAndOpacityTint();
        const FLinearColor Ink=FLinearColor(.07f,.15f,.105f)*Style.GetColorAndOpacityTint();
        auto Line=[&](TArray<FVector2D> P,FLinearColor C,float W=1.4f){ FSlateDrawElement::MakeLines(Out,Layer,G.ToPaintGeometry(),P,ESlateDrawEffect::None,C,true,W); };
        auto Circle=[&](FVector2D Center,float Radius,FLinearColor C,float W=1.f){TArray<FVector2D> P;for(int32 I=0;I<=64;++I){float A=2*PI*I/64;P.Add(Center+FVector2D(FMath::Cos(A),FMath::Sin(A))*Radius);}Line(P,C,W);};
        Circle(FVector2D(36,36),33,Gold.CopyWithNewOpacity(.35f));
        Circle(FVector2D(36,36),29,Gold.CopyWithNewOpacity(.20f));
        if(Kind==0)
        {
            for(int32 I=0;I<3;++I){float X=23+13*I,Y=I==1?44:28;Line({{X,19},{X,Y-4}},Ink,1.8f);Line({{X,Y+4},{X,53}},Ink,1.8f);Circle({X,Y},4,Gold,1.8f);}
        }
        else if(Kind==1)
        {
            Circle({36,36},18,Ink,1.5f);
            Line({{36,12},{36,18}},Gold);Line({{36,54},{36,60}},Gold);Line({{12,36},{18,36}},Gold);Line({{54,36},{60,36}},Gold);
            Line({{44,24},{40,40},{28,48},{32,32},{44,24}},Ink,1.8f);Line({{44,24},{32,32},{40,40}},Gold,1.8f);
        }
        else
        {
            Line({{36,23},{29,20},{17,21},{17,48},{28,47},{36,51},{44,47},{55,48},{55,21},{43,20},{36,23},{36,51}},Ink,1.7f);
            Line({{21,29},{29,28},{32,30}},Gold);Line({{21,35},{29,34},{32,36}},Gold);Line({{21,41},{29,40},{32,42}},Gold);
            Line({{41,29},{48,28},{51,29}},Gold);Line({{41,35},{48,34},{51,35}},Gold);Line({{41,41},{48,40},{51,41}},Gold);
        }
        return Layer;
    }
private:int32 Kind=0;
};
