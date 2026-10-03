#include "World/TripoChaseHideZone.h"
#include "World/TripoChaseNavigation.h"
#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
ATripoChaseHideZone::ATripoChaseHideZone()
{
    Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
    SetRootComponent(Volume);
    Volume->SetBoxExtent(FVector(160,160,120));
    Volume->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Volume->SetCollisionResponseToAllChannels(ECR_Ignore);
    Volume->SetCollisionResponseToChannel(ECC_GameTraceChannel1,ECR_Block);
    Volume->bDynamicObstacle=true;
    Volume->SetAreaClassOverride(UTripoHideNavArea::StaticClass());
    Volume->SetCanEverAffectNavigation(true);
    Volume->ShapeColor = FColor::Green;
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Volume);
    Label->SetRelativeLocation(FVector(0,0,150));
    Label->SetText(FText::FromString(TEXT("HIDE / Aggro down")));
    Label->SetTextRenderColor(FColor::Green);
    Label->SetWorldSize(28);
}
void ATripoChaseHideZone::OnConstruction(const FTransform& Transform)
{ Super::OnConstruction(Transform); SetEnabled(bEnabled); }
void ATripoChaseHideZone::SetEnabled(bool bNewEnabled)
{ bEnabled=bNewEnabled; Volume->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision); Volume->SetCanEverAffectNavigation(bEnabled); }
bool ATripoChaseHideZone::ContainsPoint(FVector Point) const
{
    if (!bEnabled) return false;
    const FVector Local = Volume->GetComponentTransform().InverseTransformPosition(Point);
    const FVector Extent = Volume->GetUnscaledBoxExtent();
    return FMath::Abs(Local.X)<=Extent.X && FMath::Abs(Local.Y)<=Extent.Y && FMath::Abs(Local.Z)<=Extent.Z;
}
