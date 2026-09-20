#include "World/TripoInteractorComponent.h"
UTripoInteractorComponent::UTripoInteractorComponent() { PrimaryComponentTick.bCanEverTick = false; }
bool UTripoInteractorComponent::CanTrigger(bool bAllowEcho) const
{
    return !bSuppressed && (Kind == ETripoInteractor::Player || (Kind == ETripoInteractor::Echo && bAllowEcho));
}
