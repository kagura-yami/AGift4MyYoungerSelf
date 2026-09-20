#include "Story/TripoMemoryLandmark.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoTags.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"
ATripoMemoryLandmark::ATripoMemoryLandmark()
{
    PrimaryActorTick.bCanEverTick = true;
    Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual")); SetRootComponent(Visual); Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")); if (Cube.Succeeded()) Visual->SetStaticMesh(Cube.Object);
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Reference")); Label->SetupAttachment(Visual); Label->SetRelativeLocation(FVector(0,0,150)); Label->SetWorldSize(25);
    Identity = CreateDefaultSubobject<UTripoIdentityComponent>(TEXT("Identity")); Identity->SourceType = TripoTags::SourceMemoryProjection;
}
void ATripoMemoryLandmark::OnConstruction(const FTransform& T) { Super::OnConstruction(T); Label->SetText(FText::FromString(Reference)); }
void ATripoMemoryLandmark::Tick(float Delta)
{
    Super::Tick(Delta); Visual->SetVisibility(bProjectionVisible); // Visibility never changes a gameplay condition.
}
