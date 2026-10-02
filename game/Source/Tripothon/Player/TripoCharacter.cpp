#include "Player/TripoCharacter.h"
#include "Player/TripoLocomotionAnimInstance.h"
#include "World/TripoTeleportPoint.h"
#include "DrawDebugHelpers.h"
#include "Materials/Material.h"
#include "EngineUtils.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Abilities/TripoWorldAbilities.h"
#include "Time/TripoHistoryComponent.h"
#include "Lab/TripoHUD.h"
#include "Story/TripoStoryTrigger.h"
#include "World/TripoInteractorComponent.h"
#include "World/TripoWorldSubsystem.h"
#include "World/TripoMechanism.h"
#include "Progress/TripoProgressSubsystem.h"
#include "EngineUtils.h"
#include "Core/TripoIdentityComponent.h"
#include "Core/TripoTags.h"
#include "Player/TripoMovementRules.h"
#include "Player/TripoMovementComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Engine/GameInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "UObject/ConstructorHelpers.h"

ATripoCharacter::ATripoCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UTripoMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;
    Abilities = CreateDefaultSubobject<UTripoAbilityComponent>(TEXT("Abilities"));
    Interactor = CreateDefaultSubobject<UTripoInteractorComponent>(TEXT("Interactor"));
    Interactor->Kind = ETripoInteractor::Player;
    Identity = CreateDefaultSubobject<UTripoIdentityComponent>(TEXT("Identity"));
    Identity->SourceType = TripoTags::SourcePlayer;
    History = CreateDefaultSubobject<UTripoHistoryComponent>(TEXT("History"));
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
    GetCharacterMovement()->GravityScale = 1.5f;
    GetCharacterMovement()->AirControl = .45f;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
    CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
    CameraArm->SetupAttachment(RootComponent);
    CameraArm->SetUsingAbsoluteRotation(true);
    CameraArm->bUsePawnControlRotation = true;
    CameraArm->TargetArmLength = 280.f;
    CameraArm->SetRelativeRotation(FRotator(-35.f, 0.f, 0.f));
    CameraArm->bEnableCameraLag = true;
    CameraArm->CameraLagSpeed = 12.f;
    auto* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraArm);
    // Player visual: the authored mini-character on ACharacter's built-in mesh component.
    // Gameplay collision still belongs to the capsule; this mesh is view-only.
    // The source FBX is Z-up with the face on -Y, so the left-handed import lands the face on
    // this mesh's local +Y; yaw -90 turns it onto the actor's +X forward, matching the contract
    // in docs/角色朝向镜头规则.md. Feet sit on mesh Z=0, so the mesh drops to the capsule bottom.
    if (auto* CharacterVisual = GetMesh())
    {
        static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMesh(
            TEXT("/Game/Models/juese/SK_MiniCharacter_Son_01.SK_MiniCharacter_Son_01"));
        if (CharacterMesh.Succeeded()) CharacterVisual->SetSkeletalMeshAsset(CharacterMesh.Object);
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> CharacterMaterial(TEXT("/Game/Models/lv4/caizhi1_shili.caizhi1_shili"));
        if (CharacterMaterial.Succeeded()) CharacterVisual->SetMaterial(0, CharacterMaterial.Object);
        CharacterVisual->SetAnimInstanceClass(UTripoLocomotionAnimInstance::StaticClass());
        CharacterVisual->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
        CharacterVisual->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
        CharacterVisual->SetRelativeScale3D(FVector(CharacterMeshScale));
        CharacterVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CharacterVisual->SetGenerateOverlapEvents(false);
        CharacterVisual->SetCanEverAffectNavigation(false);
    }
    StonePreview = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StonePreview"));
    StonePreview->SetupAttachment(RootComponent); StonePreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    StonePreview->SetAbsolute(true,true,true); StonePreview->SetVisibility(false); StonePreview->SetWorldScale3D(FVector(1.5,1.5,.25));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PreviewCube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (PreviewCube.Succeeded()) StonePreview->SetStaticMesh(PreviewCube.Object);
    static ConstructorHelpers::FObjectFinder<UMaterial> PreviewMaterial(TEXT("/Engine/EngineDebugMaterials/WireframeMaterial.WireframeMaterial"));
    if (PreviewMaterial.Succeeded()) StonePreview->SetMaterial(0, PreviewMaterial.Object);
    StonePreview->SetCastShadow(false);

    Mapping = CreateDefaultSubobject<UInputMappingContext>(TEXT("PlayerMapping"));
    ForwardAction = CreateDefaultSubobject<UInputAction>(TEXT("Forward"));
    RightAction = CreateDefaultSubobject<UInputAction>(TEXT("Right"));
    LookAction = CreateDefaultSubobject<UInputAction>(TEXT("Look"));
    LookPitchAction = CreateDefaultSubobject<UInputAction>(TEXT("LookPitch"));
    for (UInputAction* Action : {ForwardAction.Get(), RightAction.Get(), LookAction.Get(), LookPitchAction.Get()}) Action->ValueType = EInputActionValueType::Axis1D;
    JumpAction = CreateDefaultSubobject<UInputAction>(TEXT("Jump"));
    ResetAction = CreateDefaultSubobject<UInputAction>(TEXT("ResetPractice"));
    PauseAction = CreateDefaultSubobject<UInputAction>(TEXT("Pause"));
    InteractAction = CreateDefaultSubobject<UInputAction>(TEXT("Interact"));
    RestartAction = CreateDefaultSubobject<UInputAction>(TEXT("RestartChallenge"));
    DashAction = CreateDefaultSubobject<UInputAction>(TEXT("Dash"));
    UpAction = CreateDefaultSubobject<UInputAction>(TEXT("UpDash"));
    StoneAction = CreateDefaultSubobject<UInputAction>(TEXT("Stone"));
    SlowAction = CreateDefaultSubobject<UInputAction>(TEXT("Slow"));
    RewindAction = CreateDefaultSubobject<UInputAction>(TEXT("RewindSelf"));
    TargetRewindAction = CreateDefaultSubobject<UInputAction>(TEXT("RewindTarget"));
    EchoAction = CreateDefaultSubobject<UInputAction>(TEXT("Echo"));
    PauseAction->bTriggerWhenPaused = true;
    Mapping->MapKey(ForwardAction, EKeys::W);
    Mapping->MapKey(ForwardAction, EKeys::S).Modifiers.Add(CreateDefaultSubobject<UInputModifierNegate>(TEXT("Backward")));
    Mapping->MapKey(RightAction, EKeys::D);
    Mapping->MapKey(RightAction, EKeys::A).Modifiers.Add(CreateDefaultSubobject<UInputModifierNegate>(TEXT("Left")));
    Mapping->MapKey(LookAction, EKeys::MouseX);
    Mapping->MapKey(LookPitchAction, EKeys::MouseY);
    Mapping->MapKey(JumpAction, EKeys::SpaceBar);
    Mapping->MapKey(ResetAction, EKeys::F9);
    Mapping->MapKey(PauseAction, EKeys::Escape);
    Mapping->MapKey(InteractAction, EKeys::E);
    Mapping->MapKey(RestartAction, EKeys::BackSpace);
    Mapping->MapKey(DashAction, EKeys::LeftShift);
    Mapping->MapKey(UpAction, EKeys::LeftControl);
    Mapping->MapKey(StoneAction, EKeys::Q);
    Mapping->MapKey(SlowAction, EKeys::F);
    Mapping->MapKey(RewindAction, EKeys::R);
    Mapping->MapKey(TargetRewindAction, EKeys::T);
    Mapping->MapKey(EchoAction, EKeys::C);
}

void ATripoCharacter::BeginPlay()
{
    Super::BeginPlay();
    Abilities->AddTickPrerequisiteComponent(GetCharacterMovement());
    History->AddTickPrerequisiteComponent(GetCharacterMovement());
    PracticeStart = GetActorLocation();
    UTripoWorldSubsystem::Get(this)->SetCheckpoint(this, GetActorTransform());
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->JumpZVelocity = JumpSpeed;
    if (auto* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetControlRotation(FRotator(-35.f, GetActorRotation().Yaw, 0.f));
        if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) Subsystem->AddMappingContext(Mapping, 0);
        PC->SetInputMode(FInputModeGameOnly());
    }
}

void ATripoCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    if (auto* Enhanced = Cast<UEnhancedInputComponent>(Input))
    {
        Enhanced->BindAction(ForwardAction, ETriggerEvent::Triggered, this, &ATripoCharacter::MoveForward);
        Enhanced->BindAction(RightAction, ETriggerEvent::Triggered, this, &ATripoCharacter::MoveRight);
        Enhanced->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATripoCharacter::Look);
        Enhanced->BindAction(LookPitchAction, ETriggerEvent::Triggered, this, &ATripoCharacter::LookPitch);
        Enhanced->BindAction(JumpAction, ETriggerEvent::Started, this, &ATripoCharacter::RequestJump);
        Enhanced->BindAction(ResetAction, ETriggerEvent::Started, this, &ATripoCharacter::RequestReset);
        Enhanced->BindAction(PauseAction, ETriggerEvent::Started, this, &ATripoCharacter::TogglePause);
        Enhanced->BindAction(InteractAction, ETriggerEvent::Started, this, &ATripoCharacter::Interact);
        Enhanced->BindAction(RestartAction, ETriggerEvent::Started, this, &ATripoCharacter::RestartChallenge);
        Enhanced->BindAction(DashAction, ETriggerEvent::Started, this, &ATripoCharacter::Dash);
        Enhanced->BindAction(UpAction, ETriggerEvent::Started, this, &ATripoCharacter::UpDash);
        Enhanced->BindAction(StoneAction, ETriggerEvent::Started, this, &ATripoCharacter::PreviewStone);
        Enhanced->BindAction(StoneAction, ETriggerEvent::Completed, this, &ATripoCharacter::PlaceStone);
        Enhanced->BindAction(SlowAction, ETriggerEvent::Started, this, &ATripoCharacter::SlowTarget);
        Enhanced->BindAction(RewindAction, ETriggerEvent::Started, this, &ATripoCharacter::RewindSelf);
        Enhanced->BindAction(TargetRewindAction, ETriggerEvent::Started, this, &ATripoCharacter::RewindTarget);
        Enhanced->BindAction(EchoAction, ETriggerEvent::Started, this, &ATripoCharacter::SpawnEcho);
    }
}

void ATripoCharacter::MoveForward(const FInputActionValue& Value)
{
    if (!GameplayInputAllowed()) return;
    AddMovementInput(FRotator(0.f, GetCameraYaw(), 0.f).Vector(), Value.Get<float>());
    ++MovementEvents;
}
void ATripoCharacter::MoveRight(const FInputActionValue& Value)
{
    if (!GameplayInputAllowed()) return;
    AddMovementInput(FRotationMatrix(FRotator(0.f, GetCameraYaw(), 0.f)).GetUnitAxis(EAxis::Y), Value.Get<float>());
    ++MovementEvents;
}
void ATripoCharacter::Look(const FInputActionValue& Value)
{
    if (!GameplayInputAllowed()) return;
    if (Controller) Controller->SetControlRotation(FRotator(Controller->GetControlRotation().Pitch,
        FRotator::NormalizeAxis(Controller->GetControlRotation().Yaw + Value.Get<float>() * 1.5f), 0.f));
}
void ATripoCharacter::LookPitch(const FInputActionValue& Value)
{
    if (!GameplayInputAllowed() || !Controller) return;
    const FRotator View = Controller->GetControlRotation();
    const float Pitch = FMath::Clamp(FRotator::NormalizeAxis(View.Pitch) + Value.Get<float>() * 1.5f, MinViewPitch, MaxViewPitch);
    Controller->SetControlRotation(FRotator(Pitch, View.Yaw, 0.f));
}
void ATripoCharacter::RequestJump(const FInputActionValue&)
{
    if (!GameplayInputAllowed()) return;
    FGuid Handle;
    if (GetCharacterMovement()->IsFalling() && Abilities->GetLevel(ETripoAbility::WallJump) > 0 && Abilities->TryActivate(ETripoAbility::WallJump, nullptr, Handle) == ETripoAbilityFailure::None) return;
    JumpRequestedAt = GetWorld()->GetTimeSeconds();
}
void ATripoCharacter::Dash(const FInputActionValue&) { if (GameplayInputAllowed()) { FGuid H; Abilities->TryActivate(ETripoAbility::Dash, nullptr, H); } }
void ATripoCharacter::UpDash(const FInputActionValue&) { if (GameplayInputAllowed()) { FGuid H; Abilities->TryActivate(ETripoAbility::UpDash, nullptr, H); } }
void ATripoCharacter::PreviewStone(const FInputActionValue&) { bStonePreview = GameplayInputAllowed() && Abilities->GetLevel(ETripoAbility::StepStone) > 0; }
void ATripoCharacter::PlaceStone(const FInputActionValue&)
{
    StonePreview->SetVisibility(false);
    if (bStonePreview && GameplayInputAllowed()) { FGuid H; Abilities->TryActivate(ETripoAbility::StepStone, nullptr, H); }
    bStonePreview = false;
}
ATripoMechanism* ATripoCharacter::FindTimeTarget() const
{
    ATripoMechanism* Target = nullptr; double Best = 600*600;
    const FVector Forward = FRotator(0, GetCameraYaw(), 0).Vector();
    for (TActorIterator<ATripoMechanism> It(GetWorld()); It; ++It)
    {
        const FVector Offset = It->GetActorLocation() - GetActorLocation();
        if (It->Kind == ETripoMechanismKind::Platform && It->bTimeAffectable && Offset.SizeSquared() < Best && FVector::DotProduct(Offset.GetSafeNormal2D(), Forward) > .3)
        { Target = *It; Best = Offset.SizeSquared(); }
    }
    return Target;
}
void ATripoCharacter::SlowTarget(const FInputActionValue&) { if (GameplayInputAllowed()) { FGuid H; Abilities->TryActivate(ETripoAbility::Slow, FindTimeTarget(), H); } }
void ATripoCharacter::RewindSelf(const FInputActionValue&) { if (GameplayInputAllowed()) { FGuid H; Abilities->TryActivate(ETripoAbility::Rewind, nullptr, H); } }
void ATripoCharacter::RewindTarget(const FInputActionValue&) { if (GameplayInputAllowed()) { if (auto* Target = FindTimeTarget()) { FGuid H; Abilities->TryActivate(ETripoAbility::Rewind, Target, H); } else Abilities->ReportFailure(ETripoAbility::Rewind, ETripoAbilityFailure::NoTarget); } }
void ATripoCharacter::SpawnEcho(const FInputActionValue&) { if (GameplayInputAllowed() && !Abilities->CancelAbility(ETripoAbility::Echo)) { FGuid H; Abilities->TryActivate(ETripoAbility::Echo, nullptr, H); } }
void ATripoCharacter::RequestReset(const FInputActionValue&) { ResetPracticePosition(); }
void ATripoCharacter::Interact(const FInputActionValue&)
{
    if (!GameplayInputAllowed()) return;
    if (auto* PC = Cast<APlayerController>(Controller)) if (auto* HUD = Cast<ATripoHUD>(PC->GetHUD())) if (HUD->OpenExchange()) return;
    for (TActorIterator<ATripoStoryTrigger> It(GetWorld()); It; ++It) if (It->Interact(this)) return;
    ATripoMechanism* Nearest = nullptr; double Best = FMath::Square(250.);
    for (TActorIterator<ATripoMechanism> It(GetWorld()); It; ++It)
    {
        const double D = FVector::DistSquared(It->GetActorLocation(), GetActorLocation());
        if (It->Kind == ETripoMechanismKind::Switch && D < Best) { Best = D; Nearest = *It; }
    }
    if (Nearest) Nearest->Interact(this);
}
void ATripoCharacter::RestartChallenge(const FInputActionValue&) { if (GameplayInputAllowed()) UTripoProgressSubsystem::Get(this)->Restart(this); }
void ATripoCharacter::TogglePause(const FInputActionValue&)
{
    if (auto* Runtime = GetGameInstance()->GetSubsystem<UTripoRuntimeSubsystem>())
        Runtime->SetPauseReason(ETripoPauseReason::Menu, !Runtime->HasPauseReason(ETripoPauseReason::Menu));
}
float ATripoCharacter::GetCameraYaw() const { return Controller ? Controller->GetControlRotation().Yaw : GetActorRotation().Yaw; }
void ATripoCharacter::LookForTest(float ViewYaw)
{
#if !UE_BUILD_SHIPPING
    if (Controller) Controller->SetControlRotation(FRotator(Controller->GetControlRotation().Pitch, FRotator::NormalizeAxis(ViewYaw), 0.f));
#endif
}

void ATripoCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (auto* Progress = UTripoProgressSubsystem::Get(this); Progress && Progress->HasPendingLoad()) Progress->ApplyPendingLoad(this);
    if (!GameplayInputAllowed())
    {
        JumpRequestedAt = -1000.; TestSeconds = 0; bTestJump = false;
        bStonePreview = false; StonePreview->SetVisibility(false);
        ConsumeMovementInputVector();
        return;
    }
    TRACE_CPUPROFILER_EVENT_SCOPE(TripoCharacterTick);
    if (bStonePreview)
    {
        FTripoAbilityParameters P; FVector L;
        if (Abilities->GetParameters(ETripoAbility::StepStone, P))
        {
            const bool bValid = UTripoStoneAbility::CheckPlacement(this, P, L) == ETripoAbilityFailure::None;
            StonePreview->SetWorldLocation(L); StonePreview->SetVisibility(true);
            DrawDebugBox(GetWorld(), L, FVector(75,75,12.5), bValid ? FColor::Green : FColor::Red, false, -1, 0, 2);
        }
    }
    const double Now = GetWorld()->GetTimeSeconds();
    const bool bGrounded = GetCharacterMovement()->IsMovingOnGround();
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed *
        (GetCharacterMovement()->IsFalling() && bJumpSpent ? JumpHorizontalSpeedScale : 1.f);
    // ControlRotation owns the view. Only the body follows it; movement and body
    // rotation never write back into the camera, so strafing cannot orbit the view.
    const float ViewYaw = GetCameraYaw();
    const float Smoothed = FMath::FixedTurn(GetActorRotation().Yaw, ViewYaw, BodyTurnSpeed * DeltaSeconds);
    SetActorRotation(FRotator(0.f, TripoMovement::ClampYaw(Smoothed, ViewYaw, YawLimit), 0.f));
    if (bGrounded)
    {
        LastGroundedAt = Now;
    }
    if (GetCharacterMovement()->MovementMode != MOVE_Custom && TripoMovement::CanUseBufferedJump(Now, JumpRequestedAt, LastGroundedAt, bGrounded, bJumpSpent, JumpBufferSeconds, CoyoteSeconds))
    {
        // One impulse, also valid briefly after leaving a ledge. Landing rearms it.
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * JumpHorizontalSpeedScale;
        GetCharacterMovement()->Velocity.X *= JumpHorizontalSpeedScale;
        GetCharacterMovement()->Velocity.Y *= JumpHorizontalSpeedScale;
        GetCharacterMovement()->Velocity.Z = JumpSpeed;
        GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        JumpRequestedAt = -1000.;
        bJumpSpent = true;
        ++JumpCount;
    }
    if (GetActorLocation().Z < -800.f) UTripoWorldSubsystem::Get(this)->RestorePlayer(this);
#if !UE_BUILD_SHIPPING
    if (TestSeconds > 0.f)
    {
        TestSeconds -= DeltaSeconds;
        if (auto* PC = Cast<APlayerController>(GetController()))
        {
            if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
            {
                Subsystem->InjectInputForAction(ForwardAction, FInputActionValue(static_cast<float>(TestInput.X)), {}, {});
                Subsystem->InjectInputForAction(RightAction, FInputActionValue(static_cast<float>(TestInput.Y)), {}, {});
                if (bTestJump) { Subsystem->InjectInputForAction(JumpAction, FInputActionValue(true), {}, {}); bTestJump = false; }
            }
        }
    }
#endif
}
void ATripoCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    bJumpSpent = false;
    CastChecked<UTripoMovementComponent>(GetCharacterMovement())->ResetAirUses();
    LastGroundedAt = GetWorld()->GetTimeSeconds();
}
void ATripoCharacter::ResetPracticePosition()
{
    if (!GetWorld()->GetMapName().Contains(TEXT("L_LogicLab"))) return;
    GetCharacterMovement()->StopMovementImmediately();
    if (TeleportTo(PracticeStart, FRotator::ZeroRotator, false, false))
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Falling);
        JumpRequestedAt = LastGroundedAt = -1000.;
        bJumpSpent = false;
        TestSeconds = 0.f;
        if (Controller) Controller->SetControlRotation(FRotator(-35.f, 0.f, 0.f));
    }
}
void ATripoCharacter::ResetAfterRestore()
{
    bStonePreview = false; StonePreview->SetVisibility(false);
    CastChecked<UTripoMovementComponent>(GetCharacterMovement())->ResetAirUses();
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->SetMovementMode(MOVE_Falling);
    JumpRequestedAt = LastGroundedAt = -1000.; bJumpSpent = false; TestSeconds = 0;
    if (Controller) { Controller->SetControlRotation(FRotator(-35, GetActorRotation().Yaw, 0)); Controller->ResetIgnoreMoveInput(); }
}
bool ATripoCharacter::GameplayInputAllowed() const
{
    if (Interactor->bSuppressed) return false;
    auto* PC = Cast<APlayerController>(Controller); auto* HUD = PC ? Cast<ATripoHUD>(PC->GetHUD()) : nullptr;
    return !HUD || !HUD->IsGameplayBlocked();
}
FString ATripoCharacter::GetStonePreviewHint() const
{
    if (!bStonePreview) return TEXT("");
    FTripoAbilityParameters P; FVector L;
    return Abilities->GetParameters(ETripoAbility::StepStone, P) && UTripoStoneAbility::CheckPlacement(const_cast<ATripoCharacter*>(this), P, L) == ETripoAbilityFailure::None ? TEXT("落点可用，松开 Q 放置") : TEXT("当前落点不可放置");
}
void ATripoCharacter::DriveForTest(float Seconds, float Forward, float Right, bool bJump)
{
#if !UE_BUILD_SHIPPING
    TestSeconds = FMath::Clamp(Seconds, 0.f, 10.f);
    TestInput = FVector2D(FMath::Clamp(Forward, -1.f, 1.f), FMath::Clamp(Right, -1.f, 1.f));
    bTestJump = bJump;
#endif
}

bool ATripoCharacter::HandleForwardDisplacement()
{
    for (TActorIterator<ATripoTeleportPoint> It(GetWorld()); It; ++It)
        if (It->TryTeleport(this)) return true;
    return false;
}
