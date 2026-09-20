#include "Abilities/TripoAbilityDefinition.h"
#include "Progress/TripoChallengeDefinition.h"
#include "Story/TripoStoryCatalog.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
EDataValidationResult UTripoAbilityDefinition::IsDataValid(FDataValidationContext& Context) const
{
    if (!IsValidDefinition()) { Context.AddError(FText::FromString(TEXT("Ability requires exactly three finite, non-negative parameter levels and a valid ability ID."))); return EDataValidationResult::Invalid; }
    return EDataValidationResult::Valid;
}
EDataValidationResult UTripoChallengeDefinition::IsDataValid(FDataValidationContext& Context) const
{
    if (!IsValidDefinition()) { Context.AddError(FText::FromString(TEXT("Challenge requires an ID, a finite budget, eight level floors and unique positive-weight rewards."))); return EDataValidationResult::Invalid; }
    return EDataValidationResult::Valid;
}
EDataValidationResult UTripoStoryCatalog::IsDataValid(FDataValidationContext& Context) const
{
    if (!IsValidCatalog()) { Context.AddError(FText::FromString(TEXT("Story requires unique IDs, text, eight level floors and an acyclic prerequisite graph with no missing event."))); return EDataValidationResult::Invalid; }
    return EDataValidationResult::Valid;
}
#endif
