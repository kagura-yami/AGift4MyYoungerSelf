#include "Time/TripoEchoActor.h"
#include "World/TripoInteractorComponent.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoTags.h"
#include "Components/SkeletalMeshComponent.h"
ATripoEchoActor::ATripoEchoActor(const FObjectInitializer& Initializer) : Super(Initializer)
{
    Tags.Add(TEXT("TripoEcho"));
    Interactor->Kind = ETripoInteractor::Echo;
    Identity->SourceType = TripoTags::SourceEcho;
}
void ATripoEchoActor::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Source = Cast<ATripoCharacter>(GetOwner()))
    {
        GetMesh()->SetSkeletalMeshAsset(Source->GetMesh()->GetSkeletalMeshAsset());
        GetMesh()->SetRelativeTransform(Source->GetMesh()->GetRelativeTransform());
        GetMesh()->SetAnimInstanceClass(Source->GetMesh()->GetAnimClass());
    }
    if (auto* Material = LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/UI/Prototype/M_Echo.M_Echo")))
        for (int32 I=0; I<GetMesh()->GetNumMaterials(); ++I) GetMesh()->SetMaterial(I,Material);
}
