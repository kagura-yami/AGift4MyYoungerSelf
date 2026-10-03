#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TripoGameMode.generated.h"
UCLASS()
class TRIPOTHON_API ATripoGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ATripoGameMode();
};

/** Front end world has no playable pawn or gameplay actors. */
UCLASS()
class TRIPOTHON_API ATripoFrontEndMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ATripoFrontEndMode();
};
