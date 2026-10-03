#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "World/TripoMechanism.h"
#include "TripoWorldSubsystem.generated.h"

class ATripoCharacter;
USTRUCT()
struct FTripoCheckpoint
{
    GENERATED_BODY()
    UPROPERTY() FTransform Player;
    UPROPERTY() TMap<FGuid, FTripoMechanismState> Mechanisms;
    // Optional schema-1 field; missing keys use the level initial floor.
    UPROPERTY() TMap<FGuid, int32> ElevatorFloors;
    UPROPERTY() TArray<double> Cooldowns;
    UPROPERTY() bool bValid = false;
};
UCLASS()
class TRIPOTHON_API UTripoWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, meta=(WorldContext="Context")) static UTripoWorldSubsystem* Get(const UObject* Context);
    UFUNCTION(BlueprintCallable) bool SetCheckpoint(ATripoCharacter* Player, FTransform Transform, bool bChallengeStart = false);
    UFUNCTION(BlueprintCallable) bool RestorePlayer(ATripoCharacter* Player, bool bRestart = false);
    UFUNCTION(BlueprintPure) FString GetLastFailure() const { return LastFailure; }
    bool IsSafeSpawn(ATripoCharacter* Player, const FVector& Location) const;
private:
    UPROPERTY() FTripoCheckpoint Checkpoint;
    UPROPERTY() FTripoCheckpoint ChallengeStart;
    FString LastFailure;
};
