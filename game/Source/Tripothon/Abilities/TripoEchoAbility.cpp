#include "Abilities/TripoEchoAbility.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Player/TripoCharacter.h"
#include "Player/TripoPlayerController.h"
static ATripoPlayerController* EchoController(const UObject* Instance)
{
    auto* Component=Cast<UTripoAbilityComponent>(Instance->GetOuter());
    auto* Player=Component ? Cast<ATripoCharacter>(Component->GetOwner()) : nullptr;
    return Player ? Cast<ATripoPlayerController>(Player->GetController()) : nullptr;
}
ETripoAbilityFailure UTripoEchoAbility::Validate(AActor*, const FTripoAbilityParameters&) const
{
    auto* PC=EchoController(this);
    if (!PC) return ETripoAbilityFailure::InvalidContext;
    return PC->GetEchoCount()<PC->GetEchoCapacity() ? ETripoAbilityFailure::None : ETripoAbilityFailure::Capacity;
}
ETripoAbilityFailure UTripoEchoAbility::BeginEffect(AActor*, const FTripoAbilityParameters&)
{
    auto* PC=EchoController(this);
    return PC && PC->CreateEcho() ? ETripoAbilityFailure::None : ETripoAbilityFailure::InvalidContext;
}
