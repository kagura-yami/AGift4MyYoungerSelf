"""Create the isolated, reproducible UE tool evaluation project (not game code)."""
import json
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
EVAL = ROOT / 'tools/evaluation'
PROBE = EVAL / 'Probe'

def write(path, text):
    target = PROBE / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(text, encoding='utf-8')

write('Probe.uproject', json.dumps({
    'FileVersion': 3, 'EngineAssociation': '5.7',
    'Modules': [{'Name':'Probe','Type':'Runtime','LoadingPhase':'Default'}],
    'Plugins': [{'Name':name,'Enabled':True} for name in
        ['EnhancedInput','PythonScriptPlugin','EditorScriptingUtilities','FunctionalTestingEditor','MCPUnreal','RemoteControl']]
        + [{'Name':name,'Enabled':False} for name in ['ModelContextProtocol','UnrealMCP']]
}, indent=2))
for target, kind in [('Probe','Game'),('ProbeEditor','Editor')]:
    write(f'Source/{target}.Target.cs', f'''using UnrealBuildTool;
public class {target}Target : TargetRules {{
 public {target}Target(TargetInfo Target) : base(Target) {{
  Type = TargetType.{kind}; DefaultBuildSettings = BuildSettingsVersion.V6;
  IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
  ExtraModuleNames.Add("Probe");
 }}
}}
''')
write('Source/Probe/Probe.Build.cs', '''using UnrealBuildTool;
public class Probe : ModuleRules {
 public Probe(ReadOnlyTargetRules Target) : base(Target) {
  PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
  PublicDependencyModuleNames.AddRange(new[]{"Core","CoreUObject","Engine","InputCore","EnhancedInput"});
 }
}
''')
write('Source/Probe/Probe.cpp', '''#include "Modules/ModuleManager.h"
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Probe, "Probe");
''')
write('Source/Probe/ProbeCharacter.h', '''#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "InputActionValue.h"
#include "ProbeCharacter.generated.h"
class UInputAction;
class UInputMappingContext;
UCLASS()
class PROBE_API AProbeCharacter : public ACharacter {
 GENERATED_BODY()
public:
 AProbeCharacter();
 virtual void BeginPlay() override;
 virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
 virtual void Tick(float Delta) override;
 UFUNCTION(BlueprintCallable) void Drive(float Seconds, bool bJump);
 UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 MovementEvents = 0;
private:
 UPROPERTY() TObjectPtr<UInputAction> MoveAction;
 UPROPERTY() TObjectPtr<UInputAction> JumpAction;
 UPROPERTY() TObjectPtr<UInputMappingContext> Mapping;
 float DriveRemaining = 0;
 bool bDriveJump = false;
 void Move(const FInputActionValue& Value);
 void JumpPressed(const FInputActionValue& Value);
 void JumpReleased(const FInputActionValue& Value);
};
UCLASS()
class PROBE_API AProbeHUD : public AHUD {
 GENERATED_BODY()
public: virtual void DrawHUD() override;
};
UCLASS()
class PROBE_API AProbeGameMode : public AGameModeBase {
 GENERATED_BODY()
public: AProbeGameMode();
};
''')
write('Source/Probe/ProbeCharacter.cpp', '''#include "ProbeCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
AProbeCharacter::AProbeCharacter() {
 PrimaryActorTick.bCanEverTick = true;
 GetCapsuleComponent()->InitCapsuleSize(34,88);
 GetCharacterMovement()->MaxWalkSpeed = 300;
 GetCharacterMovement()->JumpZVelocity = 500;
 auto* Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
 Arm->SetupAttachment(RootComponent); Arm->TargetArmLength=650;
 Arm->SetRelativeRotation(FRotator(-35,0,0));
 auto* Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
 Camera->SetupAttachment(Arm);
 MoveAction=CreateDefaultSubobject<UInputAction>(TEXT("MoveAction"));
 MoveAction->ValueType=EInputActionValueType::Axis1D;
 JumpAction=CreateDefaultSubobject<UInputAction>(TEXT("JumpAction"));
 Mapping=CreateDefaultSubobject<UInputMappingContext>(TEXT("Mapping"));
 Mapping->MapKey(MoveAction,EKeys::W); Mapping->MapKey(JumpAction,EKeys::SpaceBar);
}
void AProbeCharacter::BeginPlay() {
 Super::BeginPlay();
 if(auto* PC=Cast<APlayerController>(GetController())) {
  if(auto* Sub=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) Sub->AddMappingContext(Mapping,0);
 }
}
void AProbeCharacter::SetupPlayerInputComponent(UInputComponent* Input) {
 Super::SetupPlayerInputComponent(Input);
 if(auto* E=Cast<UEnhancedInputComponent>(Input)) {
  E->BindAction(MoveAction,ETriggerEvent::Triggered,this,&AProbeCharacter::Move);
  E->BindAction(JumpAction,ETriggerEvent::Started,this,&AProbeCharacter::JumpPressed);
  E->BindAction(JumpAction,ETriggerEvent::Completed,this,&AProbeCharacter::JumpReleased);
 }
}
void AProbeCharacter::Move(const FInputActionValue& V) { AddMovementInput(FVector::ForwardVector,V.Get<float>()); ++MovementEvents; }
void AProbeCharacter::JumpPressed(const FInputActionValue&) { Jump(); }
void AProbeCharacter::JumpReleased(const FInputActionValue&) { StopJumping(); }
void AProbeCharacter::Drive(float Seconds,bool bJump) { DriveRemaining=FMath::Clamp(Seconds,0.f,10.f); bDriveJump=bJump; }
void AProbeCharacter::Tick(float Delta) {
 Super::Tick(Delta);
 if(DriveRemaining<=0) return;
 DriveRemaining-=Delta;
 if(auto* PC=Cast<APlayerController>(GetController())) {
  if(auto* Sub=ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer())) {
   Sub->InjectInputForAction(MoveAction,FInputActionValue(1.f),{},{});
   if(bDriveJump) { Sub->InjectInputForAction(JumpAction,FInputActionValue(true),{},{}); bDriveJump=false; }
  }
 }
}
void AProbeHUD::DrawHUD() {
 Super::DrawHUD();
 DrawRect(FLinearColor(0.02f,0.06f,0.12f,0.9f),20,20,620,95);
 DrawText(TEXT("TRIPOTHON TOOL PROBE | W: MOVE | SPACE: JUMP"),FLinearColor::White,35,35,nullptr,1.25f);
 if(auto* Pawn=GetOwningPawn()) DrawText(Pawn->GetActorLocation().ToString(),FLinearColor::Yellow,35,75);
}
AProbeGameMode::AProbeGameMode() { DefaultPawnClass=AProbeCharacter::StaticClass(); HUDClass=AProbeHUD::StaticClass(); }
''')
write('Source/Probe/ProbeTests.cpp', '''#include "Misc/AutomationTest.h"
#include "ProbeCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProbeDefaults,"Tripothon.Probe.Defaults",EAutomationTestFlags::EditorContext|EAutomationTestFlags::ProductFilter)
bool FProbeDefaults::RunTest(const FString&) {
 const auto* P=GetDefault<AProbeCharacter>();
 TestEqual(TEXT("Probe movement contract"),P->GetCharacterMovement()->MaxWalkSpeed,300.f);
 TestTrue(TEXT("Jump must be possible"),P->GetCharacterMovement()->JumpZVelocity>0.f);
 TestEqual(TEXT("GameMode uses test character"),GetDefault<AProbeGameMode>()->DefaultPawnClass.Get(),AProbeCharacter::StaticClass());
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProbeFailure,"Tripothon.Probe.IntentionalFailure",EAutomationTestFlags::EditorContext|EAutomationTestFlags::ProductFilter)
bool FProbeFailure::RunTest(const FString&) { AddError(TEXT("EXPECTED_PROBE_FAILURE: verify test runner detects failure")); return false; }
#endif
''')
write('Config/DefaultEngine.ini', '''[/Script/EngineSettings.GameMapsSettings]
EditorStartupMap=/Game/Probe/L_Probe
GameDefaultMap=/Game/Probe/L_Probe
GlobalDefaultGameMode=/Script/Probe.ProbeGameMode
[/Script/Engine.RendererSettings]
r.DynamicGlobalIlluminationMethod=0
r.ReflectionMethod=0
r.DefaultFeature.AutoExposure=False
[/Script/WindowsTargetPlatform.WindowsTargetSettings]
DefaultGraphicsRHI=DefaultGraphicsRHI_DX11
[/Script/Engine.Engine]
bSmoothFrameRate=False
[/Script/Engine.UserInterfaceSettings]
RenderFocusRule=Always
[/Script/UnrealEd.EditorPerformanceSettings]
bThrottleCPUWhenNotForeground=False
''')
write('Config/DefaultInput.ini', '''[/Script/Engine.InputSettings]
DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput
DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent
''')
for repo, subdir, name in [('remi','plugin','MCPUnreal'),('avatar','ModelContextProtocol','ModelContextProtocol'),('ronildo','plugin','UnrealMCP')]:
    dest=PROBE/'Plugins'/name
    if not dest.exists(): shutil.copytree(EVAL/'repos'/repo/subdir,dest)
if (PROBE/'Plugins/McpAutomationBridge').exists():
    project_path=PROBE/'Probe.uproject'
    data=json.loads(project_path.read_text(encoding='utf-8'))
    data['Plugins'].append({'Name':'McpAutomationBridge','Enabled':False})
    project_path.write_text(json.dumps(data,indent=2),encoding='utf-8')
manifest={p.name:__import__('subprocess').check_output(['git','-C',str(p),'rev-parse','HEAD'],text=True).strip() for p in (EVAL/'repos').iterdir() if (p/'.git').exists()}
(EVAL/'reports/versions.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
print(PROBE/'Probe.uproject')
