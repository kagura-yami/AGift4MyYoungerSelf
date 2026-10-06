#include "World/TripoGiftFinish.h"
#include "World/TripoGiftBox.h"
#include "Progress/TripoProgressSubsystem.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
ATripoGiftFinish::ATripoGiftFinish()
{
    Kind=ETripoZoneKind::Safe;
    PrimaryActorTick.bCanEverTick=true;
}
void ATripoGiftFinish::BeginPlay()
{
    Super::BeginPlay();
    for (const auto& G:GiftBoxes) if (IsValid(G)) { G->bEnabled=false; G->SetActorHiddenInGame(true); G->SetActorEnableCollision(false); }
}
void ATripoGiftFinish::Reveal(int32 Count,bool bBurst)
{
    ShownCount=Count;
    for (int32 I=0;I<GiftBoxes.Num();++I) if (auto* G=GiftBoxes[I].Get())
    {
        const bool Visible=I<Count;
        G->bEnabled=Visible; G->SetActorHiddenInGame(!Visible); G->SetActorEnableCollision(Visible);
        if (!Visible || !bBurst) continue;
        auto* Light=NewObject<UPointLightComponent>(this); Light->RegisterComponent();
        Light->SetWorldLocation(G->GetActorLocation()+FVector(0,0,45)); Light->SetLightColor(FLinearColor(1,.65,.2));
        Light->SetAttenuationRadius(230); Light->SetCastShadows(false); Lights.Add(Light);
        for (int32 J=0;J<18;++J)
        {
            auto* Spark=NewObject<UStaticMeshComponent>(this);
            Spark->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere")));
            Spark->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid")));
            Spark->SetCollisionEnabled(ECollisionEnabled::NoCollision); Spark->SetCastShadow(false);
            Spark->RegisterComponent(); Spark->SetWorldLocation(G->GetActorLocation()+FVector(0,0,20)); Sparks.Add(Spark);
        }
    }
    if (bBurst) BurstAt=UTripoRuntimeSubsystem::GetRuntime(this)->GetActionSeconds();
}
void ATripoGiftFinish::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* P=UTripoProgressSubsystem::Get(this); auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if (!P || !R || R->IsActionPaused()) return;
    const int32 Saved=P->GetFinishGiftCount(ChallengeId);
    if (Saved && !ShownCount) Reveal(Saved,false);
    auto* Player=Cast<ATripoCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if (!ShownCount && GiftBoxes.Num()==2 && IsValid(GiftBoxes[0]) && IsValid(GiftBoxes[1]) && Player && Contains(Player->GetActorLocation()) && P->FinishWithGifts(Player,ChallengeId))
        Reveal(P->GetFinishGiftCount(ChallengeId),true);
    if (BurstAt<0) return;
    const float T=R->GetActionSeconds()-BurstAt;
    for (int32 I=0;I<Sparks.Num();++I)
    {
        const float A=(I%18)*2*PI/18;
        const FVector Origin=GiftBoxes[I/18]->GetActorLocation()+FVector(0,0,25);
        Sparks[I]->SetWorldLocation(Origin+FVector(FMath::Cos(A)*130*T,FMath::Sin(A)*130*T,180*T-150*T*T));
        Sparks[I]->SetWorldScale3D(FVector(.055f*FMath::Max(0.f,1-T)));
    }
    for (const auto& L:Lights) L->SetIntensity(2500*FMath::Max(0.f,1-T*3));
    if (T>=1) { for (const auto& S:Sparks) S->DestroyComponent(); for (const auto& L:Lights) L->DestroyComponent(); Sparks.Empty(); Lights.Empty(); BurstAt=-1; }
}

