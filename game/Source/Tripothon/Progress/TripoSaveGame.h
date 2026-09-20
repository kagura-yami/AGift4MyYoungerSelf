#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "World/TripoMechanism.h"
#include "TripoSaveGame.generated.h"
UCLASS()
class TRIPOTHON_API UTripoSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 SchemaVersion = 1;
    UPROPERTY() int64 Sequence = 0;
    UPROPERTY() FGuid RunId;
    UPROPERTY() int32 RunSeed = 0;
    UPROPERTY() TArray<int32> Levels;
    UPROPERTY() TSet<FName> Completed;
    UPROPERTY() TSet<FName> Viewed;
    UPROPERTY() TSet<FName> Applied;
    UPROPERTY() int32 ReplyChoice = INDEX_NONE;
    UPROPERTY() TSet<FGuid> Exchanges;
    UPROPERTY() FString MapPackage;
    UPROPERTY() FTransform Spawn;
    UPROPERTY() TMap<FGuid, FTripoMechanismState> Mechanisms;
    UPROPERTY() bool bLab = false;
    bool IsValidData() const;
};
