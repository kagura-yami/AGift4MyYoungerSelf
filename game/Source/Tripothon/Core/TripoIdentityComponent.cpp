#include "Core/TripoIdentityComponent.h"
#include "Core/TripoIdentityRegistry.h"
#include "Core/TripoTags.h"
#include "Engine/World.h"

UTripoIdentityComponent::UTripoIdentityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SourceType = TripoTags::SourceEnvironment;
}

void UTripoIdentityComponent::OnRegister()
{
    Super::OnRegister();
    if (!IsTemplate() && !StableId.IsValid()) StableId = FGuid::NewGuid();
}

void UTripoIdentityComponent::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
    Super::PostDuplicate(DuplicateMode);
    if (!IsTemplate() && DuplicateMode == EDuplicateMode::Normal) StableId = FGuid::NewGuid();
}

#if WITH_EDITOR
void UTripoIdentityComponent::PostEditImport()
{
    Super::PostEditImport();
    if (!IsTemplate()) StableId = FGuid::NewGuid();
}
#endif

void UTripoIdentityComponent::BeginPlay()
{
    Super::BeginPlay();
    GetWorld()->GetSubsystem<UTripoIdentityRegistry>()->Register(this);
}

void UTripoIdentityComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (auto* Registry = GetWorld()->GetSubsystem<UTripoIdentityRegistry>()) Registry->Unregister(this);
    Super::EndPlay(Reason);
}
