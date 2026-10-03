#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TripoPlayerController.generated.h"
class ATripoCharacter;
class ATripoEchoActor;
UCLASS()
class TRIPOTHON_API ATripoPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Tripo|Echo") bool CreateEcho();
    UFUNCTION(BlueprintCallable, Category="Tripo|Echo") bool SwitchEcho();
    UFUNCTION(BlueprintCallable, Category="Tripo|Echo") bool PossessEchoBody(ATripoCharacter* Body);
    UFUNCTION(BlueprintCallable, Category="Tripo|Echo") bool ReclaimEcho(ATripoEchoActor* Body = nullptr);
    UFUNCTION(BlueprintPure, Category="Tripo|Echo") bool HasEcho() const;
    UFUNCTION(BlueprintPure, Category="Tripo|Echo") int32 GetEchoCount() const;
    UFUNCTION(BlueprintPure, Category="Tripo|Echo") int32 GetEchoCapacity() const;
    UFUNCTION(BlueprintPure, Category="Tripo|Echo") TArray<ATripoCharacter*> GetEchoBodies() const;
    UFUNCTION(BlueprintCallable, Category="Tripo|Echo") bool OpenEchoWheel(bool bReclaim = false);
    UFUNCTION(BlueprintCallable, Category="Tripo|Echo") void MoveEchoWheel(FVector2D Delta);
    UFUNCTION(BlueprintCallable, Category="Tripo|Echo") void CloseEchoWheel(bool bCommit);
    UFUNCTION(BlueprintPure, Category="Tripo|Echo") bool IsEchoWheelOpen() const { return bEchoWheelOpen; }
    bool IsReclaimWheel() const { return bReclaimWheel; }
    int32 GetWheelSelection() const { return WheelSelection; }
    FVector2D GetWheelPointer() const { return WheelPointer; }
    TArray<ATripoCharacter*> GetWheelBodies() const;
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Echo", meta=(ClampMin="0.1")) float EchoHoldSeconds = .22f;
    virtual void FlushPressedKeys() override;
    virtual void PlayerTick(float DeltaTime) override;
    virtual bool InputKey(const FInputKeyEventArgs& Params) override;
    UFUNCTION(BlueprintCallable, Category="Tripo|Validation") void KeyForTest(FKey Key, bool bPressed);
private:
    UPROPERTY() TObjectPtr<ATripoCharacter> OriginalBody;
    UPROPERTY() TArray<TObjectPtr<ATripoEchoActor>> EchoBodies;
    UPROPERTY() TArray<TObjectPtr<ATripoCharacter>> WheelBodies;
    TWeakObjectPtr<ATripoCharacter> WheelSource;
    FVector2D WheelPointer = FVector2D::ZeroVector;
    int32 WheelSelection = INDEX_NONE;
    int32 NextEchoNumber = 1;
    bool bEchoWheelOpen = false;
    bool bEchoKeyHeld = false;
    bool bReclaimKeyHeld = false;
    bool bReclaimWheel = false;
    double ReclaimPressedAt = 0;
    double EchoPressedAt = 0;
    int64 WheelEpoch = 0;
    FRotator WheelView = FRotator::ZeroRotator;
    bool CanUseEchoInput() const;
    void TapEcho();
};
