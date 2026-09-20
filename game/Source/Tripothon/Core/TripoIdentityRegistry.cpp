#include "Core/TripoIdentityRegistry.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UTripoIdentityRegistry* UTripoIdentityRegistry::GetRegistry(const UObject* WorldContextObject)
{
    auto* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    return World ? World->GetSubsystem<UTripoIdentityRegistry>() : nullptr;
}

bool UTripoIdentityRegistry::Register(UTripoIdentityComponent* Identity)
{
    if (!IsValid(Identity) || !Identity->GetStableId().IsValid()) return false;
    const FGuid Id = Identity->GetStableId();
    if (Conflicts.Contains(Id)) return false;
    if (const auto* Existing = Entries.Find(Id); Existing && Existing->IsValid() && Existing->Get() != Identity)
    {
        Conflicts.Add(Id);
        UE_LOG(LogTripoRuntime, Error, TEXT("Duplicate stable ID %s: %s / %s"), *Id.ToString(), *GetNameSafe(Existing->Get()->GetOwner()), *GetNameSafe(Identity->GetOwner()));
        return false;
    }
    Entries.Add(Id, Identity);
    return true;
}

void UTripoIdentityRegistry::Unregister(UTripoIdentityComponent* Identity)
{
    if (!Identity) return;
    const auto* Existing = Entries.Find(Identity->GetStableId());
    if (Existing && Existing->Get() == Identity) Entries.Remove(Identity->GetStableId());
}

AActor* UTripoIdentityRegistry::Resolve(FGuid Id) const
{
    if (Conflicts.Contains(Id)) return nullptr;
    const auto* Entry = Entries.Find(Id);
    return Entry && Entry->IsValid() ? Entry->Get()->GetOwner() : nullptr;
}
