#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TripoCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UTripoAbilityComponent;
class UTripoInteractorComponent;
class UTripoIdentityComponent;
class UStaticMeshComponent;
class ATripoMechanism;
class UTripoHistoryComponent;

UCLASS()
class TRIPOTHON_API ATripoCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    ATripoCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tripo|Abilities") TObjectPtr<UTripoAbilityComponent> Abilities;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UTripoInteractorComponent> Interactor;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UTripoIdentityComponent> Identity;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UTripoHistoryComponent> History;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void Landed(const FHitResult& Hit) override;
    void ResetAfterRestore();
    FString GetStonePreviewHint() const;

    UFUNCTION(BlueprintPure, Category="Tripo|Camera") float GetCameraYaw() const;
    UFUNCTION(BlueprintCallable, Category="Tripo|Validation") void LookForTest(float ViewYaw);
    UFUNCTION(BlueprintPure, Category="Tripo|Camera") float GetMovementBasisYaw() const { return GetCameraYaw(); }
    UFUNCTION(BlueprintPure, Category="Tripo|Input") int32 GetJumpCount() const { return JumpCount; }
    UFUNCTION(BlueprintPure, Category="Tripo|Input") int32 GetMovementEvents() const { return MovementEvents; }

    // Practice-map utility; does not implement checkpoint/challenge recovery.
    UFUNCTION(BlueprintCallable, Category="Tripo|Lab") void ResetPracticePosition();
    // Development only: action injection exercises Enhanced Input, never teleportation.
    UFUNCTION(BlueprintCallable, Category="Tripo|Validation") void DriveForTest(float Seconds, float Forward, float Right, bool bJump);

    UPROPERTY(EditDefaultsOnly, Category="Tripo|Movement", meta=(ClampMin="0")) float WalkSpeed = 450.f;
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Movement", meta=(ClampMin="0")) float JumpSpeed = 620.f;
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Movement", meta=(ClampMin="0")) float CoyoteSeconds = .08f;
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Movement", meta=(ClampMin="0")) float JumpBufferSeconds = .12f;
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Camera", meta=(ClampMin="0", ClampMax="90")) float YawLimit = 35.f;
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Camera", meta=(ClampMin="0")) float BodyTurnSpeed = 360.f;
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Camera", meta=(ClampMin="-89", ClampMax="0")) float MinViewPitch = -75.f;
    UPROPERTY(EditDefaultsOnly, Category="Tripo|Camera", meta=(ClampMin="0", ClampMax="89")) float MaxViewPitch = 60.f;

private:
    UPROPERTY() TObjectPtr<USpringArmComponent> CameraArm;
    UPROPERTY() TObjectPtr<UInputMappingContext> Mapping;
    UPROPERTY() TObjectPtr<UInputAction> ForwardAction;
    UPROPERTY() TObjectPtr<UInputAction> RightAction;
    UPROPERTY() TObjectPtr<UInputAction> LookAction;
    UPROPERTY() TObjectPtr<UInputAction> LookPitchAction;
    UPROPERTY() TObjectPtr<UInputAction> JumpAction;
    UPROPERTY() TObjectPtr<UInputAction> ResetAction;
    UPROPERTY() TObjectPtr<UInputAction> PauseAction;
    UPROPERTY() TObjectPtr<UInputAction> InteractAction;
    UPROPERTY() TObjectPtr<UInputAction> RestartAction;
    UPROPERTY() TObjectPtr<UInputAction> DashAction;
    UPROPERTY() TObjectPtr<UInputAction> UpAction;
    UPROPERTY() TObjectPtr<UInputAction> StoneAction;
    UPROPERTY() TObjectPtr<UInputAction> SlowAction;
    UPROPERTY() TObjectPtr<UInputAction> RewindAction;
    UPROPERTY() TObjectPtr<UInputAction> TargetRewindAction;
    UPROPERTY() TObjectPtr<UInputAction> EchoAction;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> StonePreview;
    bool bStonePreview = false;
    FVector PracticeStart;
    double LastGroundedAt = -1000.;
    double JumpRequestedAt = -1000.;
    bool bJumpSpent = false;
    int32 JumpCount = 0;
    int32 MovementEvents = 0;
    float TestSeconds = 0.f;
    FVector2D TestInput;
    bool bTestJump = false;
    void MoveForward(const FInputActionValue& Value);
    void MoveRight(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void LookPitch(const FInputActionValue& Value);
    void RequestJump(const FInputActionValue& Value);
    void RequestReset(const FInputActionValue& Value);
    void TogglePause(const FInputActionValue& Value);
    void Interact(const FInputActionValue& Value);
    void RestartChallenge(const FInputActionValue& Value);
    void Dash(const FInputActionValue& Value);
    void UpDash(const FInputActionValue& Value);
    void PreviewStone(const FInputActionValue& Value);
    void PlaceStone(const FInputActionValue& Value);
    void SlowTarget(const FInputActionValue& Value);
    void RewindSelf(const FInputActionValue& Value);
    void RewindTarget(const FInputActionValue& Value);
    void SpawnEcho(const FInputActionValue& Value);
    ATripoMechanism* FindTimeTarget() const;
    bool GameplayInputAllowed() const;
};
