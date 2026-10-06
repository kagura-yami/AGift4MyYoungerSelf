#include "Lab/TripoHUD.h"
#include "Lab/TripoMenuStyle.h"
#include "EngineUtils.h"
#include "UObject/UnrealType.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"
#include "Engine/Texture2D.h"
#include "World/TripoSchoolBooks.h"
#include "World/TripoOfficeCipher.h"
#include "Materials/MaterialInterface.h"

bool ATripoHUD::HasInventory() const { return GetInventoryKeyCount()>0 || (SchoolBooks.IsValid() && SchoolBooks->CollectedCount>0) || (InventoryCipher.IsValid() && InventoryCipher->bHasCard); }
FString ATripoHUD::GetPuzzleHint() const { return SchoolBooks.IsValid()?SchoolBooks->GetHint():FString(); }

int32 ATripoHUD::GetInventoryKeyCount() const
{
    // School's key pickup sets this flag on every matching door. Count distinct
    // key targets, not doors, so a key opening two doors is still one item.
    TSet<FName> Held;
    for(const auto& WeakDoor:InventoryDoors)
    {
        auto* Door=WeakDoor.Get(); if(!Door) continue;
        const auto* Picked=FindFProperty<FBoolProperty>(Door->GetClass(),TEXT("成功拾取钥匙"));
        const auto* Target=FindFProperty<FNameProperty>(Door->GetClass(),TEXT("门目标"));
        if(Picked && Target && Picked->GetPropertyValue_InContainer(Door))
        {
            const FName Key=Target->GetPropertyValue_InContainer(Door);
            if(!Key.IsNone()) Held.Add(Key);
        }
    }
    return Held.Num();
}
TSharedRef<SWidget> ATripoHUD::BuildInventory()
{
    InventoryDoors.Empty();
    for(TActorIterator<AActor> It(GetWorld());It;++It)
        if(FindFProperty<FBoolProperty>(It->GetClass(),TEXT("成功拾取钥匙")) && FindFProperty<FNameProperty>(It->GetClass(),TEXT("门目标"))) InventoryDoors.Add(*It);
    const TWeakObjectPtr<ATripoHUD> Self(this);
    if(auto* Icon=LoadObject<UTexture2D>(nullptr,TEXT("/Game/Materials/DoorAndKeyHint/UI_Key.UI_Key")))
    {
        UITextures.Add(Icon);
        InventoryKeyBrush.SetResourceObject(Icon);
        InventoryKeyBrush.ImageSize=FVector2D(44,44);
        InventoryKeyBrush.DrawAs=ESlateBrushDrawType::Image;
    }
    for(TActorIterator<ATripoSchoolBooks> It(GetWorld());It;++It) { SchoolBooks=*It; break; }
    auto Row=SNew(SHorizontalBox);
    for(TActorIterator<ATripoOfficeCipher> It(GetWorld());It;++It) { InventoryCipher=*It; break; }
    if(InventoryCipher.IsValid())
    {
        InventoryCipherBrush.SetResourceObject(InventoryCipher->CardUI); InventoryCipherBrush.ImageSize=FVector2D(32,57);
        Row->AddSlot().AutoWidth().Padding(0,0,18,0)
        [SNew(SHorizontalBox).Visibility_Lambda([Self]{return Self.IsValid() && Self->InventoryCipher.IsValid() && Self->InventoryCipher->bHasCard?EVisibility::HitTestInvisible:EVisibility::Collapsed;})
            +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(32).HeightOverride(57)[SNew(SImage).Image(&InventoryCipherBrush)]]
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8,0)[TripoMenu::Label(TEXT("× 1"),18,TripoMenu::Paper,false,false)]];
    }
    for(int32 I=0;I<3;++I)
    {
        if(SchoolBooks.IsValid() && SchoolBooks->Icons.IsValidIndex(I))
        {
            auto* Icon=SchoolBooks->Icons[I].Get(); UITextures.Add(Icon);
            InventoryBookBrushes[I].SetResourceObject(Icon); InventoryBookBrushes[I].ImageSize=FVector2D(36,48);
            InventoryBookBrushes[I].DrawAs=ESlateBrushDrawType::Image;
        }
        Row->AddSlot().AutoWidth().Padding(0,0,18,0)
        [SNew(SHorizontalBox).Visibility_Lambda([Self,I]{return Self.IsValid() && Self->SchoolBooks.IsValid() && Self->SchoolBooks->CollectedCount>I ? EVisibility::HitTestInvisible:EVisibility::Collapsed;})
            +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(36).HeightOverride(48)[SNew(SImage).Image(&InventoryBookBrushes[I])]]
            +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(6,0)[SNew(STextBlock).Font(TripoMenu::Font(18)).ColorAndOpacity(TripoMenu::Paper).ShadowOffset(FVector2D(1,2)).ShadowColorAndOpacity(FLinearColor::Black).Text(FText::FromString(TEXT("× 1")))]];
    }
    Row->AddSlot().AutoWidth()[SNew(SHorizontalBox)
        .Visibility_Lambda([Self]{return Self.IsValid() && Self->GetInventoryKeyCount()>0 ? EVisibility::HitTestInvisible:EVisibility::Collapsed;})
        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [SNew(SBox).WidthOverride(44).HeightOverride(44)
            [SNew(SOverlay)
                +SOverlay::Slot().Padding(2,2,0,0)[SNew(SImage).Image(&InventoryKeyBrush).ColorAndOpacity(FLinearColor(0,0,0,.65))]
                +SOverlay::Slot().Padding(0,0,2,2)[SNew(SImage).Image(&InventoryKeyBrush).ColorAndOpacity(TripoMenu::Paper)]]]
        +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10,0,0,0)
        [SNew(STextBlock).Font(TripoMenu::Font(20)).ColorAndOpacity(TripoMenu::Paper)
            .ShadowOffset(FVector2D(1,2)).ShadowColorAndOpacity(FLinearColor(0,0,0,.8))
            .Text_Lambda([Self]{return FText::FromString(FString::Printf(TEXT("× %d"),Self.IsValid()?Self->GetInventoryKeyCount():0));})]];
    return Row;
}
