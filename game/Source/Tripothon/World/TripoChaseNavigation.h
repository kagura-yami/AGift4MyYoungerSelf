#pragma once
#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "NavFilters/NavigationQueryFilter.h"
#include "TripoChaseNavigation.generated.h"
UCLASS()
class TRIPOTHON_API UTripoHideNavArea : public UNavArea
{
    GENERATED_BODY()
public:
    UTripoHideNavArea();
};
UCLASS()
class TRIPOTHON_API UTripoChaserNavFilter : public UNavigationQueryFilter
{
    GENERATED_BODY()
public:
    UTripoChaserNavFilter();
};
