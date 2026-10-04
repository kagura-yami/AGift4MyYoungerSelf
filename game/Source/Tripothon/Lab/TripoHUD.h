#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Styling/SlateBrush.h"
#include "TripoHUD.generated.h"
class SWidget;
class SBox;
class ATripoCharacter;
class UTexture2D;
struct FButtonStyle;
UCLASS()
class TRIPOTHON_API ATripoHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    /** Editor/PIE convenience: skip the story entry menu while testing a map. */
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Testing")
    bool bSkipEntryMenuInEditor = true;
    bool IsGameplayBlocked() const;
    bool OpenExchange();
    void ShowGiftReceipt(int32 AbilityIndex, int32 Level, float RevealDelay=0.f, bool bRandomDraw=false);
    UFUNCTION(BlueprintPure) bool IsGiftReceiptOpen() const { return bGiftReceipt; }
    UFUNCTION(BlueprintCallable) void HandleAction(FString Action);
    // Esc returns through tutorial/settings before releasing the menu pause.
    bool NavigateBack();
private:
    bool bFrontEnd = false;
    bool bConfirmNewGame = false;
    FString FrontEndMessage;
    TSharedRef<SWidget> BuildFrontEnd();
    enum class EMenuPage : uint8 { Pause, Settings, Tutorial, Handbook, More };
    EMenuPage MenuPage = EMenuPage::Pause;
    int32 HandbookAbility = 0;
    UPROPERTY(Transient) TArray<TObjectPtr<UTexture2D>> UITextures;
    FSlateBrush SkillBrushes[8];
    FSlateBrush PauseBrush;
    FSlateBrush BookBrush;
    FSlateBrush PaperBrush;
    FSlateBrush MainMenuArtBrush;
    FSlateBrush UpgradeBrush;
    FSlateBrush ItemBrush;
    FSlateBrush DialogueBrush;
    FSlateBrush StoryBubbleBrush;
    static const FButtonStyle& MenuButtonStyle();
    TSharedRef<SWidget> FramePanel(TSharedRef<SWidget> Content,const FSlateBrush* Art,FVector2D Size,FMargin Padding);
    FSlateBrush EchoWheelBrush;
    TSharedRef<SWidget> BuildEchoWheel();
    void InitializeUIArt();
    void UpdateStoneIndicator();
    int32 StoneCount = 0;
    int32 StoneCapacity = 0;
    double StoneWait = 0;
    double StoneLifetime = 1;
    double StoneRecoveryUntil = 0;
    bool bStoneIndicatorInitialized = false;
    TSharedRef<SWidget> BuildSkillBar();
    TSharedRef<SWidget> BuildMenuPage();
    FText SkillStatus(int32 Index) const;
    FText ChallengeText() const;
    TSharedPtr<SWidget> RootWidget;
    TSharedPtr<SBox> PanelHost;
    TSharedPtr<SBox> StoryHost;
    TSharedRef<SWidget> BuildStoryBubble();
    FString PanelKey;
    FString TimedStoryLine;
    double StoryLastTick = 0;
    float StoryLineElapsed = 0;
    bool bGiftReceipt = false;
    int32 GiftAbility = INDEX_NONE;
    int32 GiftLevel = 0;
    double GiftRevealStart = 0;
    double GiftSpinStart = 0;
    bool bGiftSpin = false;
    TSharedRef<SWidget> BuildGiftReceipt();
    bool bWasModal = false;
    bool bExchange = false;
    bool bEntryMenu = false;
    bool bCollection = false;
    bool bAbilityConfig = false;
    int32 ExchangeFrom = 0;
    int32 ExchangeTo = 1;
    FGuid ExchangeTransaction;
    FText StatusText() const;
    void RebuildPanel();
    void RefreshUI();
    ATripoCharacter* Player() const;
    bool CanConfigureAbilities() const;
};
