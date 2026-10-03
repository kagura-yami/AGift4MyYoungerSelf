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
    if (!IsValid(GetOwner())) { Destroy(); return; }
    if (bShattering)
    {
        const float Age=float(R->GetActionSeconds()-BreakStarted);
        if (Age>=.65f) { Destroy(); return; }
        const float Scale=FMath::Clamp((.65f-Age)/.25f,0.f,1.f);
        for (int32 I=0; I<Fragments.Num(); ++I)
        {
            Fragments[I]->SetWorldLocation(FragmentOrigins[I]+FragmentVelocities[I]*Age+FVector(0,0,-490.f*Age*Age));
            Fragments[I]->SetWorldRotation(FRotator((I%3-1)*180.f*Age,(I%2?1:-1)*220.f*Age,120.f*Age));
            Fragments[I]->SetWorldScale3D(FVector(.48,.48,.24)*Scale);
        }
        return;
    }
    const double Left = EndsAt - R->GetActionSeconds();
    if (Left <= 0) { Shatter(); return; }
    Label->SetText(FText::FromString(FString::Printf(TEXT("%.1fs%s"), Left, Left <= 2 ? TEXT(" !") : TEXT(""))));
    Label->SetTextRenderColor(Left <= 2 ? FColor::Red : FColor::White);
}

double ATripoStone::GetRemainingLifetime() const
{
    auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    return R ? FMath::Max(0.,EndsAt-R->GetActionSeconds()) : 0.;
}

void ATripoStone::Shatter()
{
    if (bShattering) return;
    bShattering=true;
    if (auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) BreakStarted=R->GetActionSeconds();
    Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Body->SetVisibility(false); Label->SetVisibility(false);
    // Cosmetic, collision-free fragments release the capacity immediately and never block the player.
    for (int32 Y=-1; Y<=1; ++Y) for (int32 X=-1; X<=1; ++X)
    {
        auto* Piece=NewObject<UStaticMeshComponent>(this);
        Piece->SetStaticMesh(Body->GetStaticMesh());
        Piece->SetMaterial(0,Body->GetMaterial(0));
        Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Piece->SetGenerateOverlapEvents(false); Piece->SetCanEverAffectNavigation(false);
        Piece->SetupAttachment(Body); Piece->SetAbsolute(true,true,true); Piece->RegisterComponent();
        const FVector Origin=GetActorLocation()+FVector(X*50,Y*50,0);
        Piece->SetWorldLocation(Origin); Piece->SetWorldScale3D(FVector(.48,.48,.24));
        Fragments.Add(Piece); FragmentOrigins.Add(Origin);
        FragmentVelocities.Add(FVector(X*140,Y*140,100+(X+Y+2)*18));
    }
}
