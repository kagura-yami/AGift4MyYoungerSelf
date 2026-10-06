#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "TripoTravelSubsystem.generated.h"
class SWidget;
struct FSlateDynamicImageBrush;
UCLASS()
class TRIPOTHON_API UTripoTravelSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    void Travel(FName Map);
private:
    void Loaded(UWorld* World);
    bool Update(float Dt);
    TSharedRef<SWidget> MakeScreen(bool bMovie);
    TSharedPtr<SWidget> Overlay;
    TSharedPtr<FSlateDynamicImageBrush> LoadingArt;
    FTSTicker::FDelegateHandle Ticker;
    FDelegateHandle LoadHandle;
    FName Destination;
    float Alpha=0, Elapsed=0;
    bool bOpening=false, bArrived=false;
};

