#include "Abilities/TripoStone.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"
ATripoStone::ATripoStone()
{
    PrimaryActorTick.bCanEverTick = true; Tags.Add(TEXT("TripoTemporary"));
    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body")); SetRootComponent(Body);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded()) Body->SetStaticMesh(Cube.Object);
    Body->SetCollisionProfileName(TEXT("BlockAllDynamic")); Body->SetRelativeScale3D(FVector(1.5,1.5,.25));
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Expiry")); Label->SetupAttachment(Body); Label->SetAbsolute(false,false,true);
    Label->SetRelativeLocation(FVector(0,0,100)); Label->SetWorldSize(25);
}
void ATripoStone::BeginPlay() { Super::BeginPlay(); if (auto* R = UTripoRuntimeSubsystem::GetRuntime(this)) EndsAt = R->GetActionSeconds() + Duration; }
void ATripoStone::Tick(float Delta)
{
    Super::Tick(Delta);
    auto* R = UTripoRuntimeSubsystem::GetRuntime(this); if (!R) return;
    const double Left = EndsAt - R->GetActionSeconds();
    if (Left <= 0 || !IsValid(GetOwner())) { Destroy(); return; }
    Label->SetText(FText::FromString(FString::Printf(TEXT("%.1fs%s"), Left, Left <= 2 ? TEXT(" !") : TEXT(""))));
    Label->SetTextRenderColor(Left <= 2 ? FColor::Red : FColor::White);
}
