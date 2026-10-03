#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "World/TripoMechanism.h"
#include "Progress/TripoGiftReceipt.h"
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
    // Additive schema-1 field: older saves load with an empty receipt ledger.
    UPROPERTY() TMap<FName, FTripoGiftReceipt> Gifts;
    UPROPERTY() int32 ReplyChoice = INDEX_NONE;
    UPROPERTY() TSet<FGuid> Exchanges;
    UPROPERTY() FString MapPackage;
    UPROPERTY() FTransform Spawn;
    UPROPERTY() TMap<FGuid, FTripoMechanismState> Mechanisms;
    // Optional schema-1 field; missing keys use the level initial floor.
    UPROPERTY() TMap<FGuid, int32> ElevatorFloors;
    UPROPERTY() bool bLab = false;
    bool IsValidData() const;
};
