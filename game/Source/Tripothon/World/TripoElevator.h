#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoElevator.generated.h"
class UBoxComponent;
class UTripoInteractionTarget;
class UStaticMeshComponent;
class UTripoIdentityComponent;
class ATripoCharacter;
UENUM(BlueprintType)
enum class ETripoElevatorPhase : uint8 { Docked, ClosingForTravel, Moving };

/** Single-player, two-stop lift. Cabin is a movement base; landing gates never travel. */
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoElevator : public AActor
{
    GENERATED_BODY()
public:
    ATripoElevator();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UTripoIdentityComponent> Identity;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<USceneComponent> Cabin;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UStaticMeshComponent> Floor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UStaticMeshComponent> LeftDoor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UStaticMeshComponent> RightDoor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UBoxComponent> DoorSensor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UBoxComponent> CabinVolume;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<USceneComponent> Button;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TArray<TObjectPtr<UStaticMeshComponent>> LandingDoors;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TArray<TObjectPtr<UBoxComponent>> LandingBarriers;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UTripoInteractionTarget> CabinControl;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UStaticMeshComponent> DoorButton;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TObjectPtr<UTripoInteractionTarget> DoorControl;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Elevator") TArray<TObjectPtr<UTripoInteractionTarget>> LandingControls;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door") FVector UpperDoorOffset = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door") bool bManualDoors = true;
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Elevator|Controls") TArray<TObjectPtr<AActor>> LandingButtonActors;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Controls") TArray<FVector> LandingButtonOffsets;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator", meta=(ClampMin="1")) float TravelAcceleration = 160;
    bool CanUseTarget(const UTripoInteractionTarget* Target, ATripoCharacter* Player) const;
    bool UseTarget(UTripoInteractionTarget* Target, ATripoCharacter* Player);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator", meta=(MakeEditWidget)) FVector Stop0 = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator", meta=(MakeEditWidget)) FVector Stop1 = FVector(0,0,600);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator", meta=(ClampMin="0",ClampMax="1")) int32 InitialFloor = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door") FVector LeftSlideDirection = FVector(0,-1,0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door") FVector RightSlideDirection = FVector(0,1,0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door") FVector LeftClosedPosition = FVector(-155,-50,130);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door") FVector RightClosedPosition = FVector(-155,50,130);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door") FVector LandingDoorOffset = FVector(-20,0,0);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door", meta=(ClampMin="1")) float DoorTravel = 105;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door", meta=(ClampMin=".05")) float DoorSeconds = .8f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator|Door", meta=(ClampMin="0")) float AutoCloseDelay = 1.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator", meta=(ClampMin="1")) float TravelSpeed = 200;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Elevator", meta=(ClampMin="1")) float InteractionDistance = 180;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Elevator") ETripoElevatorPhase Phase = ETripoElevatorPhase::Docked;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Elevator") int32 CurrentFloor = 0;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Elevator") float DoorAlpha = 0;
    UFUNCTION(BlueprintPure, Category="Elevator") bool CanInteract(ATripoCharacter* Player) const;
    UFUNCTION(BlueprintCallable, Category="Elevator") bool Interact(ATripoCharacter* Player);
    UFUNCTION(BlueprintCallable, Category="Elevator") void RestoreFloor(int32 SavedFloor);
    UFUNCTION(BlueprintPure, Category="Elevator") bool IsStable() const { return Phase==ETripoElevatorPhase::Docked; }
    static bool AllStable(UWorld* World);
private:
    double LastAction = 0;
    float CurrentSpeed = 0;
    void UpdateControlAnchors();
    float CloseRemaining = 0;
    int32 Destination = 0;
    bool bRestoredBeforeBegin = false;
    bool bDoorRequested = false;
    bool Contains(const UBoxComponent* Box, const ATripoCharacter* Player, bool bCapsule) const;
    bool DoorOccupied() const;
    void ApplyDoors();
};
