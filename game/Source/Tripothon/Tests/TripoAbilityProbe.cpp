#include "Tests/TripoAbilityProbe.h"
ETripoAbilityFailure UTripoAbilityProbe::Validate(AActor*, const FTripoAbilityParameters&) const
{
#if WITH_DEV_AUTOMATION_TESTS
    return ETripoAbilityFailure::None;
#else
    return ETripoAbilityFailure::NotImplemented;
#endif
}
ETripoAbilityFailure UTripoAbilityProbe::BeginEffect(AActor* Target, const FTripoAbilityParameters& Parameters)
{
    return Validate(Target, Parameters);
}
