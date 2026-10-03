#include "Lab/TripoHUD.h"
#include "Player/TripoPlayerController.h"
#include "Player/TripoCharacter.h"
#include "Time/TripoEchoActor.h"
#include "Engine/Texture2D.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/SlateRenderer.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/CoreStyle.h"

class STripoEchoWheel : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STripoEchoWheel) {}
        SLATE_ARGUMENT(TWeakObjectPtr<ATripoHUD>, HUD)
        SLATE_ARGUMENT(const FSlateBrush*, Frame)
        SLATE_ARGUMENT(const FSlateBrush*, Icon)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args) { HUD=Args._HUD; Frame=Args._Frame; Icon=Args._Icon; }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(480,500); }
    virtual bool ComputeVolatility() const override { return true; }
    virtual int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle&,bool) const override
    {
        const auto* PC=HUD.IsValid()?Cast<ATripoPlayerController>(HUD->GetOwningPlayerController()):nullptr;
        if (!PC || !PC->IsEchoWheelOpen()) return Layer;
        const auto Bodies=PC->GetWheelBodies();
        const int32 Selected=PC->GetWheelSelection();
        const FVector2D Center(240,225);
        const bool Reclaim=PC->IsReclaimWheel();
        const FLinearColor Gold=Reclaim?FLinearColor(.95,.49,.29):FLinearColor(.93,.77,.43);
        const FLinearColor Ivory(.95,.92,.8);
        auto Text=[&](const FString& Value,FVector2D Position,int32 Size,FLinearColor Color)
        {
            const auto Font=FCoreStyle::GetDefaultFontStyle("Regular",Size);
            const FVector2D Extent=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Value,Font);
            FSlateDrawElement::MakeText(Out,Layer+4,G.ToPaintGeometry(Extent,FSlateLayoutTransform(Position-Extent*.5+FVector2D(1.2,1.2))),Value,Font,ESlateDrawEffect::None,FLinearColor(0,0,0,.9));
            FSlateDrawElement::MakeText(Out,Layer+5,G.ToPaintGeometry(Extent,FSlateLayoutTransform(Position-Extent*.5)),Value,Font,ESlateDrawEffect::None,Color);
        };
        auto Arc=[&](float Radius,float Begin,float End,float Width,FLinearColor Color,int32 Z)
        {
            TArray<FVector2D> Points;
            for (int32 I=0;I<=64;++I) { const float A=FMath::Lerp(Begin,End,I/64.f); Points.Add(Center+FVector2D(FMath::Cos(A),FMath::Sin(A))*Radius); }
            FSlateDrawElement::MakeLines(Out,Layer+Z,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,Color,true,Width*G.GetAccumulatedLayoutTransform().GetScale());
        };
        // Alpha frame stays translucent over the live camera, including its open center.
        FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(FVector2D(420),FSlateLayoutTransform(Center-FVector2D(210))),Frame,ESlateDrawEffect::None,FLinearColor(1,1,1,.62));
        static const FSlateRoundedBoxBrush CenterBrush(FLinearColor::White,110.f);
        FSlateDrawElement::MakeBox(Out,Layer+1,G.ToPaintGeometry(FVector2D(210),FSlateLayoutTransform(Center-FVector2D(105))),&CenterBrush,ESlateDrawEffect::None,FLinearColor(.025,.055,.05,.68));
        const float Step=2*PI/FMath::Max(1,Bodies.Num());
        FString Selection=TEXT("移动鼠标选择");
        for (int32 I=0;I<Bodies.Num();++I)
        {
            const float A=-PI/2+I*Step;
            const bool Active=I==Selected;
            auto* Echo=Cast<ATripoEchoActor>(Bodies[I]);
            const FString Name=Echo?FString::Printf(TEXT("分身 %02d"),Echo->EchoNumber):TEXT("本体");
            if (Active) Selection=Name;
            if (Active)
            {
                Arc(164,A-Step*.46f,A+Step*.46f,61,Reclaim?FLinearColor(.22,.055,.025,.58):FLinearColor(.02,.1,.075,.58),2);
                Arc(198,A-Step*.46f,A+Step*.46f,2,Gold,3);
                Arc(129,A-Step*.46f,A+Step*.46f,1,Gold,3);
            }
            if (Bodies.Num()>1)
            {
                const float Edge=A-Step*.5f;
                TArray<FVector2D> Line={Center+FVector2D(FMath::Cos(Edge),FMath::Sin(Edge))*132,Center+FVector2D(FMath::Cos(Edge),FMath::Sin(Edge))*194};
                FSlateDrawElement::MakeLines(Out,Layer+3,G.ToPaintGeometry(),Line,ESlateDrawEffect::None,FLinearColor(.8,.72,.53,.65),true,1);
            }
            const FVector2D Point=Center+FVector2D(FMath::Cos(A),FMath::Sin(A))*162;
            FSlateDrawElement::MakeBox(Out,Layer+4,G.ToPaintGeometry(FVector2D(34),FSlateLayoutTransform(Point-FVector2D(17,27))),Icon,ESlateDrawEffect::None,Active?Gold:Ivory);
            Text(Name,Point+FVector2D(0,20),16,Active?Gold:Ivory);
            if (Bodies[I]==PC->GetPawn()) Text(TEXT("当前"),Point+FVector2D(0,39),10,Ivory);
        }
        Text(Reclaim?TEXT("回收分身"):TEXT("切换分身"),Center+FVector2D(0,-45),13,Gold);
        Text(Selection,Center+FVector2D(0,-9),19,Ivory);
        Text(Selected==INDEX_NONE?TEXT("中心松开 · 取消"):TEXT("视角预览中"),Center+FVector2D(0,24),12,Ivory);
        Text(FString::Printf(TEXT("%d / %d 分身"),PC->GetEchoCount(),PC->GetEchoCapacity()),Center+FVector2D(0,49),11,Gold);
        const FVector2D Pointer=Center+PC->GetWheelPointer();
        TArray<FVector2D> Diamond={Pointer+FVector2D(0,-5),Pointer+FVector2D(5,0),Pointer+FVector2D(0,5),Pointer+FVector2D(-5,0),Pointer+FVector2D(0,-5)};
        FSlateDrawElement::MakeLines(Out,Layer+6,G.ToPaintGeometry(),Diamond,ESlateDrawEffect::None,Gold,true,2);
        Text(Reclaim?TEXT("松开 X 回收选中分身"):TEXT("松开 C 接管选中角色"),FVector2D(240,453),16,Ivory);
        Text(TEXT("鼠标悬停预览 · 回到中心或右键取消"),FVector2D(240,480),12,Ivory);
        return Layer+6;
    }
private:
    TWeakObjectPtr<ATripoHUD> HUD;
    const FSlateBrush* Frame=nullptr;
    const FSlateBrush* Icon=nullptr;
};
TSharedRef<SWidget> ATripoHUD::BuildEchoWheel()
{
    if (auto* Texture=LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/Prototype/T_UI_EchoWheel.T_UI_EchoWheel")))
    { UITextures.Add(Texture); EchoWheelBrush.SetResourceObject(Texture); EchoWheelBrush.ImageSize=FVector2D(420); }
    else EchoWheelBrush=*FCoreStyle::Get().GetBrush("NoBrush");
    const TWeakObjectPtr<ATripoHUD> WeakThis(this);
    // Keep a stable desired size: collapsing the leaf initially leaves SScaleBox with a zero-sized layout.
    // OnPaint skips all drawing while closed; HitTestInvisible never captures gameplay input.
    return SNew(STripoEchoWheel).HUD(WeakThis).Frame(&EchoWheelBrush).Icon(&SkillBrushes[6])
        .Visibility(EVisibility::HitTestInvisible);
}
