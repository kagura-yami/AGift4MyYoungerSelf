#include "World/TripoChaseNavigation.h"
UTripoHideNavArea::UTripoHideNavArea() { DefaultCost=1; DrawColor=FColor::Green; }
UTripoChaserNavFilter::UTripoChaserNavFilter()
{
    FNavigationFilterArea Area; Area.AreaClass=UTripoHideNavArea::StaticClass(); Area.bIsExcluded=true; Areas.Add(Area);
}
