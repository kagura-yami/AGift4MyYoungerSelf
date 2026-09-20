#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TripoPlayerController.generated.h"

UCLASS()
class TRIPOTHON_API ATripoPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual bool InputKey(const FInputKeyEventArgs& Params) override;
    UFUNCTION(BlueprintCallable, Category="Tripo|Validation") void KeyForTest(FKey Key, bool bPressed);
};
