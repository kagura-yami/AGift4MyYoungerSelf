#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TripoHUD.generated.h"
class SWidget;
class SBox;
class ATripoCharacter;
UCLASS()
class TRIPOTHON_API ATripoHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    bool IsGameplayBlocked() const;
    bool OpenExchange();
    UFUNCTION(BlueprintCallable) void HandleAction(FString Action);
private:
    TSharedPtr<SWidget> RootWidget;
    TSharedPtr<SBox> PanelHost;
    FString PanelKey;
    bool bWasModal = false;
    bool bExchange = false;
    bool bEntryMenu = false;
    bool bCollection = false;
    int32 ExchangeFrom = 0;
    int32 ExchangeTo = 1;
    FGuid ExchangeTransaction;
    FText StatusText() const;
    void RebuildPanel();
    void RefreshUI();
    ATripoCharacter* Player() const;
};
